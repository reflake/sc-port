#define SDL_MAIN_HANDLED

#include <algorithm>
#include <array>
#include <CascLib.h>
#include <cassert>
#include <commdlg.h> // windows only
#include <cstring>
#include <fileapi.h>
#include <handleapi.h>
#include <iostream>
#include <memory>
#include <minwindef.h>
#include <stdexcept>
#include <unistd.h>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <windows.h>

#include <intrin.h>

#include <sstream>

#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include <boost/format/format_fwd.hpp>

#include <SDL_events.h>
#include <SDL_hints.h>
#include <SDL_keycode.h>
#include <SDL_pixels.h>
#include <SDL_rect.h>
#include <SDL_stdinc.h>
#include <SDL_surface.h>
#include <SDL_syswm.h>
#include <SDL_video.h>
#include <SDL_timer.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL.h>

#include <audio/Music.hpp>
#include <data/Assets.hpp>
#include <data/Common.hpp>
#include <data/Grp.hpp>
#include <data/Images.hpp>
#include <filesystem/MpqArchive.hpp>
#include <filesystem/Storage.hpp>

#include <vulkan/Api.hpp>

#include "data/Tile.hpp"
#include "data/Map.hpp"
#include "data/Sprite.hpp"
#include "data/TextStrings.hpp"
#include "data/Tileset.hpp"
#include "script/IScriptEngine.hpp"
#include "entity/ScriptedDoodad.hpp"

#include "vulkan/VulkanGraphics.hpp"
#include <diagnostic/Clock.hpp>

using boost::format;

using data::Grp;
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
using data::ImagesTable;
using	data::SpriteTable;

using data::EntryName;

using entity::ScriptedDoodad;

using filesystem::Storage;

using script::IScriptEngine;

using VulkanGraphics = renderer::vulkan::Graphics;

using renderer::DrawableHandle;
 
struct SpriteAtlas;

void throwSdlError(const char* msg)
{
	auto err = format("%1%: %2%") % msg % SDL_GetError();

	throw runtime_error( err.str() );
}

void initSDL()
{
	if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO) < 0)
	{
		throwSdlError("Couldn't initialize SDL library");
	}
}

struct App {
	SDL_Renderer* renderer = nullptr;
	SDL_Window*	  window = nullptr;
	SDL_Surface*  screenSurface = nullptr;

	TilesetData    tilesetData;

	DrawableHandle tilesetView;

	IScriptEngine                        scriptEngine;
	vector<shared_ptr<ScriptedDoodad>>   scriptedDoodads;

	shared_ptr<renderer::A_Graphics> graphics;
	data::Assets assets;

	unordered_map<data::grpID, DrawableHandle> loadedSprites;

	tileID tiles[256][256];

	audio::MusicPlayer musicPlayer;

	double   tickRate = 16.0 * 1.5;
	uint64_t currentTick = 0;

	double nextGameTick     = 0.0;
	double realTime         = 0.0;
	double deltaTime        = 0.0;
	double averageDeltaTime = 0.0;
	double nextFpsMeasure   = 0.0;
	double fps;
};

void freeWindow(App&);

const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 960;

void createWindow(App& app)
{
	const int windowFlags = SDL_WINDOW_SHOWN | SDL_WINDOW_VULKAN;

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

void initializeGraphicsAPI(App& app)
{
	app.graphics = renderer::vulkan::CreateGraphics(app.window, &app.assets);
}

void initializeMusicPlayer(App& app)
{
	app.musicPlayer = audio::MusicPlayer(&app.assets);
	app.musicPlayer.Initialize();
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

	memset(app.tiles, 0, sizeof(app.tiles));

	for(int i = 0; i < mapInfo.dimensions.x; i++)
	for(int j = 0; j < mapInfo.dimensions.y; j++)
	{
		if (app.tiles[i][j] != 0)
			continue;

		auto [tileGroupID, variation] = mapInfo.GetTile(i, j);
		auto tileGroup = app.tilesetData.tileGroups[tileGroupID];
		tileID tileID;

		if (app.tilesetData.IsDoodad(tileGroupID))
		{
			tileID = tileGroup.terrain.variations[variation];
		}
		else
		{
			tileID = tileGroup.doodad.tiles[variation];
		}

		app.tiles[i][j] = tileID;
	}
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

void drawMap(MapInfo &mapInfo, App &app, const position pos) {

	if (mapInfo.dimensions.x == 0 || mapInfo.dimensions.y == 0)
		return;

	{
		Clock clock("BeginRendering()");

		app.graphics->BeginRendering();
		app.graphics->SetTilesetPalette(app.tilesetData.palette);
		app.graphics->SetView(pos);
	}

	int leftBorderIndex  = std::max<int>(0, pos.x / TILE_SIZE);
	int rightBorderIndex = std::min<int>(mapInfo.dimensions.x, (pos.x + SCREEN_WIDTH) / TILE_SIZE + 1);
	int upBorderIndex    = std::max<int>(0, pos.y / TILE_SIZE);
	int downBorderIndex  = std::min<int>(mapInfo.dimensions.y, (pos.y + SCREEN_HEIGHT) / TILE_SIZE + 1);

	{
		Clock clock("RenderTiles()");

		for (int x = leftBorderIndex; x < rightBorderIndex; x++)
		for (int y = upBorderIndex; y < downBorderIndex; y++)
		{
			tileID tileId = app.tiles[x][y];

			app.graphics->Draw(app.tilesetView, tileId, { x * TILE_SIZE, y * TILE_SIZE });
		};
	}

	{
		Clock clock("RenderSprites()");
		for(auto& doodad : app.scriptedDoodads)
		{
			auto grpID = doodad->grpID;
			auto frame = doodad->GetCurrentFrame();
			auto spriteSheet = app.loadedSprites[grpID];

			app.graphics->Draw(spriteSheet, frame, doodad->pos);
		}
	}

	app.graphics->PresentToScreen();
}

void processInput(glm::vec<2, double>& pos, int &move, double deltaTime)
{
	const float speed = 1000.0f;
	float x = 0, y = 0;

	if ((move & Up) > 0)
		y -= 1;
	if ((move & Down) > 0)
		y += 1;
	if ((move & Left) > 0)
		x -= 1;
	if ((move & Right) > 0)
		x += 1;

	pos.x += x * speed * deltaTime;
	pos.y += y * speed * deltaTime;
}

void placeScriptedDoodads(
	App& app, Storage& storage, MapInfo& mapInfo)
{
	SpriteTable spriteTable;
	data::ReadSpriteTable(storage, spriteTable);

	ImagesTable imagesTable;
	data::ReadImagesTable(storage, imagesTable);

	script::ReadIScriptFile(storage, "scripts/iscript.bin", app.scriptEngine);

	app.scriptEngine.Clear();
	app.scriptEngine.Init();
	
	app.scriptedDoodads.clear();

	for(auto& doodad : mapInfo.sprites)
	{
		auto& imageID  = spriteTable.imageID[doodad.spriteID];
		auto  grpID    = imagesTable.grpID[imageID] - 1;
		auto  scriptID = imagesTable.iScriptID[imageID];
		auto  pos      = doodad.position;

		auto instance = std::make_shared<ScriptedDoodad>(scriptID, grpID, pos);

		app.scriptEngine.RunScriptableObject(instance);
		app.scriptedDoodads.push_back(instance);
	}
}

void loadTileset(App& app, MapInfo& mapInfo, Storage& storage, data::Tileset tileset)
{
	if (app.tilesetView != nullptr)
	{
		app.graphics->FreeDrawable(app.tilesetView);
		app.tilesetView = nullptr;
	}

	vector<bool> usedTiles(app.tilesetData.GetTileCount(), false);

	for(int i = 0; i < mapInfo.dimensions.x; i++)
	for(int j = 0; j < mapInfo.dimensions.y; j++)
	{
		tileID tileID = app.tilesetData.GetMappedIndex(app.tiles[i][j]);

		usedTiles[tileID] = true;
	};

	app.tilesetView = app.graphics->LoadTileset(app.tilesetData, usedTiles);
}

void loadDoodadGrps(App& app, Storage& storage)
{
	data::TextStringsTable imageStrings;
	data::ReadTextStringsTable(storage, "arr/images.tbl", imageStrings);

	for(auto& doodad : app.scriptedDoodads)
	{
		uint8_t pixels[data::GRP_SPRITE_SQUARE_LIMIT];

		if (app.loadedSprites.contains(doodad->grpID))
			continue;

		auto grpPath = imageStrings.entries[doodad->grpID];
		auto grp = Grp::ReadGrpFile(storage, grpPath);

		app.loadedSprites[doodad->grpID] = app.graphics->LoadSpriteSheet(grp);
	}
}

bool tryOpenMap(App& app, const char* mapPath, Storage& storage, MapInfo& mapInfo)
{
	app.graphics->WaitIdle();
	
	try
	{

		loadMap(app, mapPath, storage, mapInfo);

		for(auto& [_, spriteSheet] : app.loadedSprites)
		{
			app.graphics->FreeDrawable(spriteSheet);
		}

		app.loadedSprites.clear();

		loadTileset(app, mapInfo, storage, mapInfo.tileset);
		placeScriptedDoodads(app, storage, mapInfo);
		loadDoodadGrps(app, storage);

		app.scriptEngine.Process();

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

void gameTick(App& app, MapInfo& mapInfo)
{
	Clock clock("GameTick()");

	for(; app.nextGameTick < app.realTime; app.nextGameTick += 1.0 / app.tickRate)
	{
		app.scriptEngine.PlayNextFrame();

		const int waterCycleFrequence = 3;

		if (data::HasTileSetWater(mapInfo.tileset) && (app.currentTick % waterCycleFrequence) == 0) {

			app.tilesetData.palette.cyclePaletteColor<1, 6>();
			app.tilesetData.palette.cyclePaletteColor<7, 7>();
		}

		app.currentTick++;
	}
}

int main(int argc, char *argv[]) {

	auto storagePath = argv[1];

	Storage storage(storagePath);
	App app;

	app.assets = data::Assets(&storage);
	
	initSDL();
	createWindow(app);
	initializeGraphicsAPI(app);
	initializeMusicPlayer(app);

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

	int                 moveInput = 0;
	glm::vec<2, double> viewPos = { 0, 0 };

	uint64_t counterCurrent, counterLast = SDL_GetPerformanceCounter();

	app.musicPlayer.Play();

	while (running) {

		Clock clock("Loop()");

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
				case SDLK_p:
					ShowClockReports();
					break;
				case SDLK_o: {

					uint64_t diff = SDL_GetPerformanceCounter();

					app.musicPlayer.Stop();

					if (showOpenDialog(mapPath, sizeof(mapPath), hwnd)) {

						openedMapSuccessfully = tryOpenMap(app, mapPath, storage, mapInfo);

						if (!openedMapSuccessfully)

							showLoadErrorMessage(hwnd, mapPath);

						else
						{
							app.realTime     = 0.0;
							app.currentTick  = 0;
							app.nextGameTick = 0;
						}

						viewPos = { 0, 0 };
					}

					diff = SDL_GetPerformanceCounter() - diff;

					counterLast += diff;

					app.musicPlayer.Play();
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

		drawMap(mapInfo, app, viewPos);
		processInput(viewPos, moveInput, app.deltaTime);
		gameTick(app, mapInfo);

		clock.Stop();

		usleep(10000);
		
		counterCurrent = SDL_GetPerformanceCounter();
		app.deltaTime = static_cast<double>(counterCurrent - counterLast) / SDL_GetPerformanceFrequency();
		app.realTime += app.deltaTime;
		app.averageDeltaTime = (app.averageDeltaTime + app.deltaTime) * 0.5;

		counterLast = counterCurrent;

		if (app.nextFpsMeasure < app.realTime)
		{
			app.fps            = (app.fps + 1.0 / app.averageDeltaTime) * 0.5;
			app.nextFpsMeasure = app.realTime + 1.0;
		}

		boost::format time("Gauss Engine: %.2f (+%.5f) FPS: %.2f");
		time % app.realTime % app.averageDeltaTime % app.fps;

		SDL_SetWindowTitle(app.window, time.str().c_str());

		app.musicPlayer.Process();
	};

	app.graphics->WaitIdle();
	app.graphics->Release();

	freeWindow(app);

	return 0;
}