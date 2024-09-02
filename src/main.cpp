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

#include <SDL_events.h>
#include <SDL_hints.h>
#include <SDL_keycode.h>
#include <SDL_pixels.h>
#include <SDL_rect.h>
#include <SDL_surface.h>
#include <SDL_video.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL.h>

#include "data/Palette.hpp"
#include "data/Tile.hpp"
#include "filesystem/storage.hpp"
#include "filesystem/storage_file.hpp"

using boost::format;

using std::cout;
using std::runtime_error;
using data::Tile;
using data::MegaTile;

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

struct Page
{
	SDL_Surface* surface;
};

template<int L = 3>
void createMegaTilePages(App& app, MegaTile* megaTiles, int megaTileCount, Tile* tiles, std::vector<Page>& pages, data::Palette paletteData)
{
	auto pal = SDL_AllocPalette(256);
	auto colors = reinterpret_cast<const SDL_Color*>(paletteData.GetColors());

	SDL_SetPaletteColors(pal, colors, 0, 256);

	for(int i = 0; i < megaTileCount; i+=L*L)
	{
		const int width = L * 32;
		const int height = L * 32;
		auto surface = SDL_CreateRGBSurface(0, width, height, 8, 0, 0, 0, 0);
		SDL_SetSurfacePalette(surface, pal);

		SDL_LockSurface(surface);

		for(int j = 0; j < width; j+=8)
		for(int k = 0; k < height; k++)
		{
			int megaTileIndex = i + j / 32 + k / 32 * L;

			if (megaTileIndex >= megaTileCount)
				continue;

			auto& megaTile = megaTiles[megaTileIndex];

			int tileId = megaTile.GetTileId(k / 8 % 4, j / 8 % 4);

			auto& tile = tiles[tileId];

			auto pixel = reinterpret_cast<uint8_t*>(surface->pixels) + j + k * width;

			memcpy(pixel, &tile.palPixels[(k % 8) * 8], 8);
		}

		SDL_UnlockSurface(surface);

		auto optimizedSurface = SDL_ConvertSurface(surface, app.screenSurface->format, 0);

		SDL_FreeSurface(surface);

		pages.push_back(Page(optimizedSurface));
	}
}

int main(int argc, char* argv[])
{
	std::vector<Page> pages;

	App app;

	initSDL();
	createWindow(app);
	
	{
		filesystem::Storage storage(argv[1]);

		// Read palette of tile set
		data::WpeData wpeData;
		storage.Read("TileSet/ashworld.wpe", wpeData);

		data::Palette palette(wpeData);

		// Read mini tile's data
		filesystem::StorageFile tileSetFile;
		storage.Open("TileSet/ashworld.vr4", tileSetFile);
		
		int tileDataSize = tileSetFile.GetFileSize();
		int tileCount = tileDataSize / sizeof(Tile);
		auto tiles = new Tile[tileCount];

		tileSetFile.Read(tiles, tileDataSize);

		// Read mega tile's data
		filesystem::StorageFile megaTileSetFile;
		storage.Open("TileSet/ashworld.vx4ex", megaTileSetFile);

		int megaTileDataSize = megaTileSetFile.GetFileSize();
		int megaTileCount = megaTileDataSize / sizeof(MegaTile);
		auto megaTiles = new MegaTile[megaTileCount];

		megaTileSetFile.Read(megaTiles, megaTileDataSize);

		// Prepare images to draw
		createMegaTilePages<9>(app, megaTiles, megaTileCount, tiles, pages, palette);

		delete[] tiles;
	}

	SDL_Event event;

	bool running = true;

	int page = 0;
				
	SDL_Rect destRect { .x = 0, .y = 0, .w = 512, .h = 512 };

	while(running)
	while(SDL_PollEvent(&event))
	{
		switch(event.type)
		{
			case SDL_KEYDOWN:
				if (event.key.keysym.sym == SDLK_UP)
				{
					page++;
				}
				else if (event.key.keysym.sym == SDLK_DOWN)
				{
					page--;
				}

				page = (page + pages.size()) % pages.size();

				SDL_BlitScaled(pages[page].surface, NULL, app.screenSurface, &destRect);
				SDL_UpdateWindowSurface(app.window);
				break;

			case SDL_QUIT:
				running = false;
				break;

			default:
				break;
		}
	};

	freeWindow(app);

	return 0;
}