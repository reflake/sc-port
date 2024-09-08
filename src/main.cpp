#include "data/Grp.hpp"
#include "data/Images.hpp"
#include "data/Sprite.hpp"
#include "data/TextStrings.hpp"
#include "render/Palette.hpp"
#include "render/Tileset.hpp"
#include "script/IScriptEngine.hpp"
#include <SDL_stdinc.h>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#define SDL_MAIN_HANDLED

#include <intrin.h>

#include <algorithm>
#include <boost/format.hpp>
#include <boost/format/format_fwd.hpp>
#include <boost/algorithm/string.hpp>
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
#include "entity/ScriptedDoodad.hpp"

using boost::format;

using std::pair;
using std::runtime_error;
using std::string;
using std::vector;
using std::shared_ptr;
using std::unordered_map;

using data::TILE_SIZE;
using data::tileID;
using data::tileGroupID;
using data::tileVariation;

using data::MapInfo;
using data::TilesetData;
using data::position;
using data::TextStringsTable;
using	data::SpriteTable;
using	data::ImagesTable;
using data::Grp;

using data::EntryName;

using entity::ScriptedDoodad;

using filesystem::Storage;

using render::GridAtlas;
using render::cyclePaletteColor;
using render::createTilesetAtlas;

using script::IScriptEngine;
 
struct SpriteAtlas;

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

	TilesetData    tilesetData;
	GridAtlas      tilesetAtlas;

	IScriptEngine                        scriptEngine;
	vector<shared_ptr<ScriptedDoodad>>   scriptedDoodads;
	unordered_map<uint32_t, SpriteAtlas> spriteAtlases;
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

void loadMap(App& app, const string& mapPath, Storage& storage, MapInfo& mapInfo)
{
	filesystem::MpqArchive mapFile(mapPath.c_str());

	data::ReadMap(mapFile, mapInfo);
	data::LoadTilesetData(storage, mapInfo.tileset, app.tilesetData);
}

enum Move : int { Up = 0x01, Down = 0x02, Left = 0x04, Right = 0x08 };

struct SpriteAtlas
{
	SDL_Surface**    surfaces;
	vector<SDL_Rect> rects;
	int frames = 0;

	void Free()
	{
		rects.clear();

		if (surfaces == nullptr)
			return;

		for(int i = 0; i < frames; i++)
		{
			SDL_FreeSurface(surfaces[i]);
		}

		delete[] surfaces;
		surfaces = nullptr;
	}
};

void drawMap(MapInfo &mapInfo, App &app,
							 int &waterCycle, position& pos) {

	if (mapInfo.dimensions.x == 0 || mapInfo.dimensions.y == 0)
		return;

	int leftBorderIndex  = std::max<int>(0, pos.x / TILE_SIZE);
	int rightBorderIndex = std::min<int>(mapInfo.dimensions.x, (pos.x + SCREEN_WIDTH) / TILE_SIZE + 1);
	int upBorderIndex    = std::max<int>(0, pos.y / TILE_SIZE);
	int downBorderIndex  = std::min<int>(mapInfo.dimensions.y, (pos.y + SCREEN_HEIGHT) / TILE_SIZE + 1);

	for (int x = leftBorderIndex; x < rightBorderIndex; x++)
	for (int y = upBorderIndex; y < downBorderIndex; y++) {

		auto mapTile = mapInfo.GetTile(x, y);
		auto tileGroup = app.tilesetData.tileGroups[std::get<tileGroupID>(mapTile)];

		tileID tileId;

		if ((tileGroup.type & data::Terrain) > 0) {

			tileId = tileGroup.terrain.variations[std::get<tileVariation>(mapTile)];
		} else {

			tileId = tileGroup.doodad.tiles[std::get<tileVariation>(mapTile)];
		}

		auto tile = app.tilesetAtlas.GetTile(tileId);

		SDL_Rect destRect{.x = x * TILE_SIZE - static_cast<int>(pos.x),
											.y = y * TILE_SIZE - static_cast<int>(pos.y),
											.w = TILE_SIZE,
											.h = TILE_SIZE};

		SDL_BlitSurface(tile.first, &tile.second, app.screenSurface, &destRect);
	}

	for(auto& doodad : app.scriptedDoodads)
	{
		auto spriteAtlas = app.spriteAtlases[doodad->grpID];
		auto frameIndex  = doodad->GetCurrentFrame();
		auto surface     = spriteAtlas.surfaces[frameIndex];
		auto destRect    = spriteAtlas.rects[frameIndex];

		destRect.x += doodad->pos.x - static_cast<int>(pos.x);
		destRect.y += doodad->pos.y - static_cast<int>(pos.y);

		SDL_BlitSurface(surface, nullptr, app.screenSurface, &destRect);
	}

	SDL_UpdateWindowSurface(app.window);

	if (data::HasTileSetWater(mapInfo.tileset) && (waterCycle++ % 10) == 0) {

		cyclePaletteColor<1, 6>(app.tilesetAtlas);
		cyclePaletteColor<7, 7>(app.tilesetAtlas);
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

void placeScriptedDoodads(
	App& app, Storage& storage, MapInfo& mapInfo, TilesetData& tilesetData)
{
	SpriteTable spriteTable;
	data::ReadSpriteTable(storage, spriteTable);

	ImagesTable imagesTable;
	data::ReadImagesTable(storage, imagesTable);

	script::ReadIScriptFile(storage, "scripts/iscript.bin", app.scriptEngine);

	app.scriptEngine.Init();
	app.scriptedDoodads.clear();

	for(auto& doodad : mapInfo.sprites)
	{
		auto& imageID  = spriteTable.imageID[doodad.spriteID];
		auto  grpID    = imagesTable.grpID[imageID] - 1;
		auto  scriptID = imagesTable.iScriptID[imageID];
		auto  pos = doodad.position;

		auto instance = std::make_shared<ScriptedDoodad>(scriptID, grpID, pos);

		app.scriptEngine.RunScriptableObject(instance);
		app.scriptedDoodads.push_back(instance);
	}
}

void createDoodadAtlas(App& app, Storage& storage)
{
	TextStringsTable imageStrings;
	data::ReadTextStringsTable(storage, "arr/images.tbl", imageStrings);
	uint8_t pixels[256 * 256];

	for(auto& doodad : app.scriptedDoodads)
	{
		if (app.spriteAtlases.contains(doodad->grpID))
			continue;

		auto grpPath = imageStrings.entries[doodad->grpID];

		Grp grp;
		Grp::ReadGrpFile(storage, grpPath, grp);

		SpriteAtlas atlas;

		atlas.frames = grp.GetHeader().frameAmount;
		atlas.surfaces = new SDL_Surface*[atlas.frames];

		int frameIndex = 0;

		for(auto& frame : grp.GetFrames())
		{
			SDL_Surface* surface = SDL_CreateRGBSurface(0, frame.dimensions.x, frame.dimensions.y, 8, 0, 0, 0, 0);
			SDL_SetSurfacePalette(surface, app.tilesetAtlas.palette);

			SDL_LockSurface(surface);

			grp.GetFramePixels(frameIndex, pixels);

			int width = frame.dimensions.x;
			auto surfacePixels = reinterpret_cast<uint8_t*>(surface->pixels);

			for(int i = 0; i < surface->h; i++)
			{
				memcpy(surfacePixels + i * surface->pitch, pixels + i * width, width);
			}

			SDL_UnlockSurface(surface);
			SDL_SetColorKey(surface, SDL_TRUE, 0x00000000);

			atlas.rects.push_back(SDL_Rect { 
				.x = frame.posOffset.x - grp.GetHeader().dimensions.x / 2, .y = frame.posOffset.y - grp.GetHeader().dimensions.y / 2,
				.w = frame.dimensions.x, .h = frame.dimensions.y });
			atlas.surfaces[frameIndex++] = surface;
		}

		app.spriteAtlases[doodad->grpID] = atlas;
	}
}

bool tryOpenMap(App& app, const char* mapPath, Storage& storage, MapInfo& mapInfo)
{
	try
	{
		loadMap(app, mapPath, storage, mapInfo);

		app.tilesetAtlas.Free();

		for(auto& pair : app.spriteAtlases)
		{
			auto& spriteAtlas = std::get<SpriteAtlas>(pair);
			spriteAtlas.Free();
		}

		app.spriteAtlases.clear();

		createTilesetAtlas<10>(app.tilesetData, app.tilesetAtlas);
		placeScriptedDoodads(app, storage, mapInfo, app.tilesetData);
		createDoodadAtlas(app, storage);

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

	auto storagePath = argv[1];

	Storage storage(storagePath);

	App app;
	
	initSDL();
	createWindow(app);


	SDL_SysWMinfo wmInfo;
	SDL_VERSION(&wmInfo.version);
	SDL_GetWindowWMInfo(app.window, &wmInfo);
	HWND hwnd = wmInfo.info.win.window;


	char     mapPath[260];
	auto     ignoredMapEntries = vector<EntryName> { EntryName::Terrain_Editor };
	MapInfo  mapInfo(ignoredMapEntries);
	bool     openedMapSuccessfully = false;

	if (argc > 2) {
		// Read map
		auto mapPath = argv[2];

		openedMapSuccessfully = tryOpenMap(app, mapPath, storage, mapInfo);
		if (!openedMapSuccessfully)

			showLoadErrorMessage(hwnd, mapPath);
	}

	while(!openedMapSuccessfully)
	{
		if (showOpenDialog(mapPath, sizeof(mapPath), hwnd))
		{
			openedMapSuccessfully = tryOpenMap(app, mapPath, storage, mapInfo);

			if (!openedMapSuccessfully)
			
				showLoadErrorMessage(hwnd, mapPath);
		}
	}

	SDL_Event event;

	bool running = true;

	int      waterCycle = 0;
	int      moveInput = 0;
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

						openedMapSuccessfully = tryOpenMap(app, mapPath, storage, mapInfo);

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

		drawMap(mapInfo, app, waterCycle, viewPos);
		processInput(viewPos, moveInput);

		app.scriptEngine.PlayNextFrame();

		usleep(16000);
	};

	app.tilesetAtlas.Free();

	freeWindow(app);

	return 0;
}