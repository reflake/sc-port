#define SDL_MAIN_HANDLED

#include <intrin.h>

#include <algorithm>
#include <boost/format.hpp>
#include <boost/format/format_fwd.hpp>
#include <CascLib.h>
#include <cassert>
#include <cstring>
#include <fileapi.h>
#include <handleapi.h>
#include <iostream>
#include <minwindef.h>
#include <stdexcept>
#include <memory>
#include <unistd.h>

#include <SDL_events.h>
#include <SDL_hints.h>
#include <SDL_keycode.h>
#include <SDL_pixels.h>
#include <SDL_rect.h>
#include <SDL_surface.h>
#include <SDL_video.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL.h>

#include "data/Common.hpp"
#include "data/Map.hpp"
#include "data/Palette.hpp"
#include "data/Tile.hpp"
#include "filesystem/MpqArchive.hpp"
#include "filesystem/MpqFile.hpp"
#include "filesystem/Storage.hpp"
#include "filesystem/StorageFile.hpp"

using boost::format;

using std::pair;
using std::runtime_error;
using std::string;

using data::TILE_SIZE;
using data::tileID;
using data::tileGroupID;
using data::tileVariation;

using data::MapInfo;
using data::TilesetData;

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

const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 960;

void createWindow(App& app)
{
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
			.x = (id % dimension) * TILE_SIZE, 
			.y = id / dimension * TILE_SIZE,
			.w = TILE_SIZE, 
			.h = TILE_SIZE };

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
void createTileAtlas(App& app, TilesetData& tilesetData, Atlas& atlas)
{
	auto palette = SDL_AllocPalette(256);
	auto colors = reinterpret_cast<const SDL_Color*>(tilesetData.paletteData.GetColors());

	SDL_SetPaletteColors(palette, colors, 0, 256);

	atlas = Atlas(L, palette);

	for(int i = 0; i < tilesetData.tilesCount; i+=L*L)
	{
		const int width = L * TILE_SIZE;
		const int height = L * TILE_SIZE;
		auto surface = SDL_CreateRGBSurface(0, width, height, 8, 0, 0, 0, 0);
		SDL_SetSurfacePalette(surface, palette);

		SDL_LockSurface(surface);

		for(int j = 0; j < width; j+=8)
		for(int k = 0; k < height; k++)
		{
			tileID tileId = i + j / TILE_SIZE + k / TILE_SIZE * L;

			if (tileId >= tilesetData.tilesCount)
				continue;

			auto& tile = tilesetData.tiles[tileId];

			uint32_t chipId = tile.GetChipId(k / 8 % 4, j / 8 % 4);
			auto&    chip = tilesetData.chips[chipId];

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
	TilesetData tilesetData;

	{
		auto storagePath = argv[1];

		filesystem::Storage storage(storagePath);

		// Read map
		auto mapPath = argv[2];

		filesystem::MpqArchive mapFile(mapPath);

		data::ReadMap(mapFile, mapInfo, false);
		data::loadTilesetData(storage, mapInfo.tileset, tilesetData);
	}

	App app;
	Atlas atlas;

	initSDL();
	createWindow(app);
	createTileAtlas<10>(app, tilesetData, atlas);
	
	SDL_Event event;

	bool running = true;

	enum Move : int
	{
		Up = 0x01, Down = 0x02, Left = 0x04, Right = 0x08
	};

	int waterCycle = 0;
	int move = 0;
	float X, Y;

	while(running)
	{
		while(SDL_PollEvent(&event))
		{
			switch(event.type)
			{
				case SDL_KEYDOWN:
					switch(event.key.keysym.sym)
					{
						case SDLK_UP: move |= Up; break;
						case SDLK_DOWN: move |= Down; break;
						case SDLK_LEFT: move |= Left; break;
						case SDLK_RIGHT: move |= Right; break;
					}
					break;

				case SDL_KEYUP:
					switch(event.key.keysym.sym)
					{
						case SDLK_UP: move &= ~Up; break;
						case SDLK_DOWN: move &= ~Down; break;
						case SDLK_LEFT: move &= ~Left; break;
						case SDLK_RIGHT: move &= ~Right; break;
					}
					break;
					
				case SDL_QUIT:
					running = false;
					break;

				default:
					break;
			}
		};

		int leftBorderIndex = std::max<int>(0, X / TILE_SIZE);
		int rightBorderIndex = std::min<int>(mapInfo.dimensions.x, (X + SCREEN_WIDTH) / TILE_SIZE + 1);
		int upBorderIndex = std::max<int>(0, Y / TILE_SIZE);
		int downBorderIndex = std::min<int>(mapInfo.dimensions.y, (Y + SCREEN_HEIGHT) / TILE_SIZE + 1);
		
		for(int x = leftBorderIndex; x < rightBorderIndex; x++)
		for(int y = upBorderIndex; y < downBorderIndex; y++)
		{
			auto mapTile = mapInfo.GetTile(x, y);
			auto tileGroup = tilesetData.tileGroups[std::get<tileGroupID>(mapTile)];

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

			SDL_Rect destRect { .x = x * TILE_SIZE - (int)X, .y = y * TILE_SIZE - (int)Y, 
													.w = TILE_SIZE, .h = TILE_SIZE };

			SDL_BlitSurface(tile.first, &tile.second, app.screenSurface, &destRect);
		}

		const float speed = 12.0f;
		float x = 0, y = 0;

		if ((move & Up) > 0)
			y -= 1;
		if ((move & Down) > 0)
			y += 1;
		if ((move & Left) > 0)
			x -= 1;
		if ((move & Right) > 0)
			x += 1;

		X += x * speed;
		Y += y * speed;

		SDL_UpdateWindowSurface(app.window);

		if (data::HasTileSetWater(mapInfo.tileset) && (waterCycle++ % 10) == 0)
		{
			cyclePaletteColor<1, 6>(atlas);
			cyclePaletteColor<7, 7>(atlas);
		}

		usleep(16000);
	};
	
	atlas.Free();

	freeWindow(app);

	return 0;
}