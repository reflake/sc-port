#include "data/Common.hpp"
#include <cassert>
#include <unistd.h>
#define SDL_MAIN_HANDLED

#include <intrin.h>

#include <boost/format.hpp>
#include <boost/format/format_fwd.hpp>
#include <CascLib.h>
#include <cstring>
#include <fileapi.h>
#include <handleapi.h>
#include <iostream>
#include <minwindef.h>
#include <stdexcept>
#include <memory>

#include <SDL_events.h>
#include <SDL_hints.h>
#include <SDL_keycode.h>
#include <SDL_pixels.h>
#include <SDL_rect.h>
#include <SDL_surface.h>
#include <SDL_video.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL.h>

#include "data/Map.hpp"
#include "data/Palette.hpp"
#include "data/Tile.hpp"
#include "filesystem/MpqArchive.hpp"
#include "filesystem/MpqFile.hpp"
#include "filesystem/Storage.hpp"
#include "filesystem/StorageFile.hpp"

using boost::format;

using std::cout;
using std::runtime_error;
using std::string;
using std::make_shared;
using std::shared_ptr;
using std::pair;

using data::tileID;
using data::MEGA_TILE_SIZE;
using data::tileGroupID;
using data::tileVariation;
using data::MapInfo;
using data::Palette;
using data::Chip;
using data::Tile;
using data::TileGroup;
using data::GroupTypeFlags;

void throwSdlError(const char* msg)
{
	auto err = format("%1%: %2%") % msg % SDL_GetError();

	throw runtime_error( err.str() );
}


void initSDL()
{
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		throwSdlError("Couldn't initialize SDL library");
	}
}

struct App {
	SDL_Renderer* renderer = nullptr;
	SDL_Window*	  window = nullptr;
	SDL_Surface*  screenSurface = nullptr;
};

void freeWindow(App&);

void createWindow(App& app)
{
	const int SCREEN_WIDTH = 1280;
	const int SCREEN_HEIGHT = 960;

	const int rendererFlags = SDL_RENDERER_ACCELERATED;
	const int windowFlags   = 0;

	freeWindow(app);

	app.window = SDL_CreateWindow(
		"Game", 
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 
		SCREEN_WIDTH, SCREEN_HEIGHT, 
		windowFlags
	);

	if (!app.window)
	{
		throwSdlError("Failed to create the window");
	}

	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");

	app.renderer = SDL_CreateRenderer(app.window, -1, rendererFlags);

	if (!app.renderer)
	{
		throwSdlError("Failed to create the renderer");
	}

	app.screenSurface = SDL_GetWindowSurface(app.window);

	if (!app.screenSurface)
	{
		throwSdlError("Couldn't get the screen of the window");
	}
	
	SDL_FillRect( app.screenSurface, NULL, SDL_MapRGB( app.screenSurface->format, 0xFF, 0xFF, 0xFF ) );
}

void freeWindow(App& app)
{
	if (app.renderer)
	{
		SDL_DestroyRenderer(app.renderer);
	}

	if (app.window)
	{
		SDL_DestroyWindow(app.window);
	}
}

struct Atlas
{
	int dimension;
	SDL_Palette* palette;
	std::vector<SDL_Surface*> pages;

	pair<SDL_Surface*, SDL_Rect> GetTile(uint16_t id)
	{
		int pageIndex = id / dimension / dimension;

		id = id - pageIndex * dimension * dimension;

		SDL_Rect rect = { 
			.x = (id % dimension) * MEGA_TILE_SIZE, 
			.y = id / dimension * MEGA_TILE_SIZE,
			.w = MEGA_TILE_SIZE, 
			.h = MEGA_TILE_SIZE };

		return { pages[pageIndex], rect };
	}

	void Free()
	{
		for(auto page : pages)
		{
			SDL_FreeSurface(page);
		}

		SDL_FreePalette(palette);
	}
};

template<int L>
void createTileAtlas(App& app, Tile* tiles, int tileCount, Chip* chips, Atlas& atlas, Palette paletteData)
{
	auto palette = SDL_AllocPalette(256);
	auto colors = reinterpret_cast<const SDL_Color*>(paletteData.GetColors());

	SDL_SetPaletteColors(palette, colors, 0, 256);

	atlas = Atlas(L, palette);

	for(int i = 0; i < tileCount; i+=L*L)
	{
		const int width = L * MEGA_TILE_SIZE;
		const int height = L * MEGA_TILE_SIZE;
		auto surface = SDL_CreateRGBSurface(0, width, height, 8, 0, 0, 0, 0);
		SDL_SetSurfacePalette(surface, palette);

		SDL_LockSurface(surface);

		for(int j = 0; j < width; j+=8)
		for(int k = 0; k < height; k++)
		{
			tileID tileId = i + j / MEGA_TILE_SIZE + k / MEGA_TILE_SIZE * L;

			if (tileId >= tileCount)
				continue;

			auto& tile = tiles[tileId];

			uint32_t chipId = tile.GetChipId(k / 8 % 4, j / 8 % 4);
			auto&    chip = chips[chipId];

			auto pixels = reinterpret_cast<uint8_t*>(surface->pixels) + j + k * width;

			if (tile.IsTileMirrored(k / 8 % 4, j / 8 % 4))
			{
				for(int n = 0; n < 8; n++)

					pixels[7 - n] = chip.palPixels[(k % 8) * 8 + n];
			}
			else
			{
				memcpy(pixels, &chip.palPixels[(k % 8) * 8], 8);
			}
		}

		SDL_UnlockSurface(surface);

		atlas.pages.push_back(surface);
	}
}

template<int S, int L>
void cyclePaletteColor(Atlas& atlas)
{
	// animate water
	SDL_Color newPaletteColor[L];
	memcpy(newPaletteColor + 1, atlas.palette->colors + S, (L - 1) * sizeof(SDL_Color));

	newPaletteColor[0] = atlas.palette->colors[S + L - 1];

	SDL_SetPaletteColors(atlas.palette, newPaletteColor, S, L);
}

int main(int argc, char* argv[])
{
	MapInfo mapInfo;
	Palette palette;
	shared_ptr<Chip[]> chips;
	shared_ptr<Tile[]> tiles;
	shared_ptr<TileGroup[]> tileGroups;
	int tilesCount;

	{
		auto storagePath = argv[1];

		filesystem::Storage storage(storagePath);

		// Read map
		auto mapPath = argv[2];

		filesystem::MpqArchive mapFile(mapPath);
		filesystem::MpqFile scenarioFile;

		mapFile.Open("staredit\\scenario.chk", scenarioFile);

		int bytesAmount = scenarioFile.GetFileSize();
		auto scenarioBytes = make_shared<uint8_t[]>(bytesAmount);

		scenarioFile.Read(scenarioBytes.get(), bytesAmount);

		data::ReadMap(scenarioBytes, bytesAmount, mapInfo, false);

		// Loading palette
		string tileSetName = data::tileSetNameMap[mapInfo.tileset];

		data::WpeData wpeData;
		storage.Read(format("TileSet/%1%.wpe") % tileSetName, wpeData);

		palette = Palette(wpeData);

		// Read mini tile's data
		filesystem::StorageFile chipSetFile;
		storage.Open(format("TileSet/%1%.vr4") % tileSetName, chipSetFile);
		
		int chipDataSize = chipSetFile.GetFileSize();
		int chipCount = chipDataSize / sizeof(Tile);

		chips = make_shared<Chip[]>(chipCount);
		chipSetFile.Read(chips.get(), chipDataSize);

		// Read mega tile's data
		filesystem::StorageFile tileSetFile;
		storage.Open(format("TileSet/%1%.vx4ex") % tileSetName, tileSetFile);

		int tileDataSize = tileSetFile.GetFileSize();
		tilesCount = tileDataSize / sizeof(Tile);

		tiles = make_shared<Tile[]>(tilesCount);
		tileSetFile.Read(tiles.get(), tileDataSize);

		// Read tile groups
		filesystem::StorageFile tileGroupFile;
		storage.Open(format("TileSet/%1%.cv5") % tileSetName, tileGroupFile);

		int tileGroupDataSize = tileGroupFile.GetFileSize();
		int tileGroupCount = tileGroupDataSize / sizeof(TileGroup);

		tileGroups = make_shared<TileGroup[]>(tileGroupCount);
		tileGroupFile.Read(tileGroups.get(), tileGroupDataSize);
	}

	App app;
	Atlas atlas;

	initSDL();
	createWindow(app);
	createTileAtlas<10>(app, tiles.get(), tilesCount, chips.get(), atlas, palette);
	
	SDL_Event event;

	bool running = true;

	while(running)
	{
		while(SDL_PollEvent(&event))
		{
			switch(event.type)
			{
				case SDL_KEYDOWN:
					break;
					
				case SDL_QUIT:
					running = false;
					break;

				default:
					break;
			}
		};
		
					
		for(int x = 0; x < 60; x++)
		for(int y = 0; y < 40; y++)
		{
			auto mapTile = mapInfo.GetTile(x, y);
			auto tileGroup = tileGroups[std::get<tileGroupID>(mapTile)];

			tileID tileId;

			if ((tileGroup.type & data::Terrain) > 0)
			{
				tileId = tileGroup.terrain.variations[std::get<tileVariation>(mapTile)];
			}
			else
			{
				tileId = tileGroup.doodad.tiles[std::get<tileVariation>(mapTile)];
			}

			auto tile = atlas.GetTile(tileId);

			SDL_Rect destRect { .x = x * MEGA_TILE_SIZE, .y = y * MEGA_TILE_SIZE, 
													.w = MEGA_TILE_SIZE, .h = MEGA_TILE_SIZE };

			SDL_BlitSurface(tile.first, &tile.second, app.screenSurface, &destRect);
		}

		SDL_UpdateWindowSurface(app.window);

		if (data::HasTileSetWater(mapInfo.tileset))
		{
			cyclePaletteColor<1, 6>(atlas);
			cyclePaletteColor<7, 7>(atlas);
		}

		usleep(160000);
	};
	

	atlas.Free();

	freeWindow(app);

	return 0;
}