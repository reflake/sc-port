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

using data::MEGA_TILE_SIZE;
using data::Tile;
using data::MegaTile;
using data::tileGroupID;
using data::tileVariation;
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

	pair<SDL_Surface*, SDL_Rect> GetMegaTile(uint16_t id)
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
void createMegaTileAtlas(App& app, MegaTile* megaTiles, int megaTileCount, Tile* tiles, Atlas& atlas, data::Palette paletteData)
{
	auto palette = SDL_AllocPalette(256);
	auto colors = reinterpret_cast<const SDL_Color*>(paletteData.GetColors());

	SDL_SetPaletteColors(palette, colors, 0, 256);

	atlas = Atlas(L, palette);

	for(int i = 0; i < megaTileCount; i+=L*L)
	{
		const int width = L * MEGA_TILE_SIZE;
		const int height = L * MEGA_TILE_SIZE;
		auto surface = SDL_CreateRGBSurface(0, width, height, 8, 0, 0, 0, 0);
		SDL_SetSurfacePalette(surface, palette);

		SDL_LockSurface(surface);

		for(int j = 0; j < width; j+=8)
		for(int k = 0; k < height; k++)
		{
			int megaTileIndex = i + j / MEGA_TILE_SIZE + k / MEGA_TILE_SIZE * L;

			if (megaTileIndex >= megaTileCount)
				continue;

			auto& megaTile = megaTiles[megaTileIndex];

			int tileId = megaTile.GetTileId(k / 8 % 4, j / 8 % 4);

			auto& tile = tiles[tileId];

			auto pixels = reinterpret_cast<uint8_t*>(surface->pixels) + j + k * width;

			if (megaTile.IsTileMirrored(k / 8 % 4, j / 8 % 4))
			{
				for(int n = 0; n < 8; n++)

					pixels[7 - n] = tile.palPixels[(k % 8) * 8 + n];
			}
			else
			{
				memcpy(pixels, &tile.palPixels[(k % 8) * 8], 8);
			}
		}

		SDL_UnlockSurface(surface);

		//auto optimizedSurface = SDL_ConvertSurface(surface, app.screenSurface->format, 0);
		//SDL_FreeSurface(surface);

		atlas.pages.push_back(surface);
	}
}

int main(int argc, char* argv[])
{
	data::MapInfo mapInfo;
	data::Palette palette;
	shared_ptr<Tile[]> tiles;
	shared_ptr<MegaTile[]> megaTiles;
	shared_ptr<data::TileGroup[]> tileGroups;
	int megaTilesCount;
	Atlas atlas;

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

		palette = data::Palette(wpeData);

		// Read mini tile's data
		filesystem::StorageFile tileSetFile;
		storage.Open(format("TileSet/%1%.vr4") % tileSetName, tileSetFile);
		
		int tileDataSize = tileSetFile.GetFileSize();
		int tileCount = tileDataSize / sizeof(Tile);

		tiles = make_shared<Tile[]>(tileCount);
		tileSetFile.Read(tiles.get(), tileDataSize);

		// Read mega tile's data
		filesystem::StorageFile megaTileSetFile;
		storage.Open(format("TileSet/%1%.vx4ex") % tileSetName, megaTileSetFile);

		int megaTileDataSize = megaTileSetFile.GetFileSize();
		megaTilesCount = megaTileDataSize / sizeof(MegaTile);

		megaTiles = make_shared<MegaTile[]>(megaTilesCount);
		megaTileSetFile.Read(megaTiles.get(), megaTileDataSize);

		// Read tile groups
		filesystem::StorageFile tileGroupFile;
		storage.Open(format("TileSet/%1%.cv5") % tileSetName, tileGroupFile);

		int tileGroupDataSize = tileGroupFile.GetFileSize();
		int tileGroupCount = tileGroupDataSize / sizeof(data::TileGroup);

		tileGroups = make_shared<data::TileGroup[]>(tileGroupCount);
		tileGroupFile.Read(tileGroups.get(), tileGroupDataSize);
	}

	App app;

	initSDL();
	createWindow(app);
	createMegaTileAtlas<10>(app, megaTiles.get(), megaTilesCount, tiles.get(), atlas, palette);
	
	SDL_Event event;

	bool running = true;

	while(running)
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
				
		for(int x = 0; x < 60; x++)
		for(int y = 0; y < 40; y++)
		{
			auto mapTile = mapInfo.GetTile(x, y);
			auto group = tileGroups[std::get<tileGroupID>(mapTile)];

			data::megaTileID tileId;

			if ((group.type & data::Terrain) > 0)
			{
				tileId = group.terrain.variations[std::get<tileVariation>(mapTile)];
			}
			else
			{
				tileId = group.doodad.tiles[std::get<tileVariation>(mapTile)];
			}

			auto tile = atlas.GetMegaTile(tileId);

			SDL_Rect destRect { .x = x * MEGA_TILE_SIZE, .y = y * MEGA_TILE_SIZE, 
													.w = MEGA_TILE_SIZE, .h = MEGA_TILE_SIZE };

			SDL_BlitSurface(tile.first, &tile.second, app.screenSurface, &destRect);
		}

		SDL_UpdateWindowSurface(app.window);

		usleep(10000);
	};

	atlas.Free();

	freeWindow(app);

	return 0;
}