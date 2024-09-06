#include "render/Palette.hpp"
#include "render/Tileset.hpp"
#define SDL_MAIN_HANDLED

#include <intrin.h>

#include <algorithm>
#include <boost/format.hpp>
#include <boost/format/format_fwd.hpp>
#include <CascLib.h>
#include <cassert>
#include <cstring>
#include <commdlg.h> // windows only
#include <fileapi.h>
#include <handleapi.h>
#include <iostream>
#include <minwindef.h>
#include <stdexcept>
#include <memory>
#include <unistd.h>
#include <windows.h>

#include <SDL_events.h>
#include <SDL_hints.h>
#include <SDL_keycode.h>
#include <SDL_pixels.h>
#include <SDL_rect.h>
#include <SDL_surface.h>
#include <SDL_video.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL.h>
#include <SDL_syswm.h>

#include "data/Common.hpp"
#include "data/Map.hpp"
#include "data/Palette.hpp"
#include "data/Tile.hpp"
#include "filesystem/MpqArchive.hpp"
#include "filesystem/Storage.hpp"
#include "render/Atlas.hpp"

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
using data::position;

using render::GridAtlas;
using render::cyclePaletteColor;
using render::createTilesetAtlas;

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

bool showOpenDialog(char* out, int size, HWND hwnd)
{
	OPENFILENAME ofn;

	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hwnd;
	ofn.lpstrFile = out;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = size;
	ofn.lpstrFilter = "StarCraft maps (*.SCM; *.SCX)\0*.SCM;*.SCX\0All (*.*)\0*.*\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	if (GetOpenFileName(&ofn))
	{
		return true;
	}
	else
	{
		return false;
	}
}

void loadMap(const string& mapPath, filesystem::Storage& storage, MapInfo& mapInfo, TilesetData& tilesetData)
{
	filesystem::MpqArchive mapFile(mapPath.c_str());

	data::ReadMap(mapFile, mapInfo, false);
	data::LoadTilesetData(storage, mapInfo.tileset, tilesetData);
}

enum Move : int { Up = 0x01, Down = 0x02, Left = 0x04, Right = 0x08 };

void drawMap(MapInfo &mapInfo, TilesetData &tilesetData, App &app,
							 GridAtlas &atlas, int &waterCycle, position& pos) {

	if (mapInfo.dimensions.x == 0 || mapInfo.dimensions.y == 0)
		return;

	int leftBorderIndex = std::max<int>(0, pos.x / TILE_SIZE);
	int rightBorderIndex =
			std::min<int>(mapInfo.dimensions.x, (pos.x + SCREEN_WIDTH) / TILE_SIZE + 1);
	int upBorderIndex = std::max<int>(0, pos.y / TILE_SIZE);
	int downBorderIndex =
			std::min<int>(mapInfo.dimensions.y, (pos.y + SCREEN_HEIGHT) / TILE_SIZE + 1);

	for (int x = leftBorderIndex; x < rightBorderIndex; x++)
	for (int y = upBorderIndex; y < downBorderIndex; y++) {

		auto mapTile = mapInfo.GetTile(x, y);
		auto tileGroup = tilesetData.tileGroups[std::get<tileGroupID>(mapTile)];

		tileID tileId;

		if ((tileGroup.type & data::Terrain) > 0) {

			tileId = tileGroup.terrain.variations[std::get<tileVariation>(mapTile)];
		} else {

			tileId = tileGroup.doodad.tiles[std::get<tileVariation>(mapTile)];
		}

		auto tile = atlas.GetTile(tileId);

		SDL_Rect destRect{.x = x * TILE_SIZE - static_cast<int>(pos.x),
											.y = y * TILE_SIZE - static_cast<int>(pos.y),
											.w = TILE_SIZE,
											.h = TILE_SIZE};

		SDL_BlitSurface(tile.first, &tile.second, app.screenSurface, &destRect);
	}

	SDL_UpdateWindowSurface(app.window);

	if (data::HasTileSetWater(mapInfo.tileset) && (waterCycle++ % 10) == 0) {
		cyclePaletteColor<1, 6>(atlas);
		cyclePaletteColor<7, 7>(atlas);
	}
}

void processInput(position& pos, int &move)
{
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

	pos.x += x * speed;
	pos.y += y * speed;
}

bool tryOpenMap(const char* mapPath, filesystem::Storage& storage, 
								MapInfo& mapInfo, TilesetData& tilesetData, GridAtlas& tilesetAtlas)
{
	try
	{
		loadMap(mapPath, storage, mapInfo, tilesetData);

		tilesetAtlas.Free();
		createTilesetAtlas<10>(tilesetData, tilesetAtlas);

		return true;
	}
	catch (...)
	{
		std::cout << "Couldn't load file " << mapPath << std::endl;
		return false;
	}
}

void showLoadErrorMessage(HWND hwnd, const char* mapName)
{
	auto msg = format("Couldn't load map %1%") % mapName;

	MessageBoxA(hwnd, msg.str().c_str(), "Map load", MB_ICONWARNING | MB_OK);
}

int main(int argc, char *argv[]) {
	MapInfo mapInfo;
	TilesetData tilesetData;

	auto storagePath = argv[1];

	filesystem::Storage storage(storagePath);

	App app;
	GridAtlas tilesetAtlas;
	char mapPath[260];

	initSDL();
	createWindow(app);

	SDL_SysWMinfo wmInfo;
	SDL_VERSION(&wmInfo.version);
	SDL_GetWindowWMInfo(app.window, &wmInfo);
	HWND hwnd = wmInfo.info.win.window;

	bool openedMapSuccessfully = false;

	if (argc > 2) {
		// Read map
		auto mapPath = argv[2];

		openedMapSuccessfully = tryOpenMap(mapPath, storage, mapInfo, tilesetData, tilesetAtlas);

		if (!openedMapSuccessfully)

			showLoadErrorMessage(hwnd, mapPath);
	}

	while(!openedMapSuccessfully)
	{
		if (showOpenDialog(mapPath, sizeof(mapPath), hwnd))
		{
			openedMapSuccessfully = tryOpenMap(mapPath, storage, mapInfo, tilesetData, tilesetAtlas);

			if (!openedMapSuccessfully)
			
				showLoadErrorMessage(hwnd, mapPath);
		}
	}

	SDL_Event event;

	bool running = true;

	int waterCycle = 0;
	int moveInput = 0;
	position viewPos;

	while (running) {
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_KEYDOWN:
				switch (event.key.keysym.sym) {
				case SDLK_UP:
					moveInput |= Up;
					break;
				case SDLK_DOWN:
					moveInput |= Down;
					break;
				case SDLK_LEFT:
					moveInput |= Left;
					break;
				case SDLK_RIGHT:
					moveInput |= Right;
					break;
				case SDLK_o: {

					if (showOpenDialog(mapPath, sizeof(mapPath), hwnd)) {

						openedMapSuccessfully = tryOpenMap(mapPath, storage, mapInfo, tilesetData, tilesetAtlas);

						if (!openedMapSuccessfully)

							showLoadErrorMessage(hwnd, mapPath);

						viewPos = { 0, 0 };
					}
				} break;
				}
				break;

			case SDL_KEYUP:
				switch (event.key.keysym.sym) {
				case SDLK_UP:
					moveInput &= ~Up;
					break;
				case SDLK_DOWN:
					moveInput &= ~Down;
					break;
				case SDLK_LEFT:
					moveInput &= ~Left;
					break;
				case SDLK_RIGHT:
					moveInput &= ~Right;
					break;
				}
				break;

			case SDL_QUIT:
				running = false;
				break;

			default:
				break;
			}
		};

		drawMap(mapInfo, tilesetData, app, tilesetAtlas,
							waterCycle, viewPos);
		processInput(viewPos, moveInput);

		usleep(16000);
	};

	tilesetAtlas.Free();

	freeWindow(app);

	return 0;
}