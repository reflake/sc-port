#define SDL_MAIN_HANDLED

#include "audio/Music.hpp"
#include "A_Graphics.hpp"
#include "data/TextStrings.hpp"
#include "diagnostic/Clock.hpp"
#include "meta/PortraitTable.hpp"
#include "meta/SfxTable.hpp"
#include "meta/UnitTable.hpp"
#include "view/UnitTransmission.hpp"
#include "vulkan/Api.hpp"
#include <stdexcept>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <memory>

#include <SDL3/SDL.h>
#include <SDL3/SDL_keycode.h>
#include <SDL3/SDL_timer.h>
#include <SDL3/SDL_video.h>

#include <audio/AudioManager.hpp>
#include "data/Assets.hpp"
#include "video/Video.hpp"

#include <boost/foreach.hpp>
#include <boost/format/format_fwd.hpp>

#include <filesystem/Storage.hpp>

#include <Loop.hpp>

using boost::format;
using std::runtime_error;

void throwSdlError(const char* msg)
{
	auto err = format("%1%: %2%") % msg % SDL_GetError();

	throw runtime_error( err.str() );
}

struct App
{
	audio::AudioManager audioManager;

	std::shared_ptr<renderer::A_Graphics> graphics;
	SDL_Window* window = nullptr;
	data::Assets assets;
};

void initSDL()
{
	if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO) < 0)
	{
		throwSdlError("Couldn't initialize SDL library");
	}
}

void freeWindow(App& app)
{
	if (app.window)
	{
		SDL_DestroyWindow(app.window);
	}
}

void initializeGraphicsAPI(App& app)
{
	app.graphics = renderer::vulkan::CreateGraphics(app.window, &app.assets);
}

void createWindow(App& app)
{
	const SDL_WindowFlags windowFlags = SDL_WINDOW_VULKAN;
	const int SCREEN_WIDTH = 1280;
	const int SCREEN_HEIGHT = 960;

	freeWindow(app);

	app.window = SDL_CreateWindow(
		"Game",
		SCREEN_WIDTH, SCREEN_HEIGHT, 
		windowFlags
	);

	if (!app.window)
	{
		throwSdlError("Failed to create the window");
	}
}

// Unit transmission test
int main(int argc, char *argv[])
{
	using filesystem::Storage;
	using data::Assets;
	using data::AssetHandle;

	if (argc < 2)
	{
		std::cout << "Not enought arguments" << std::endl;
		return 1;
	}

	App app;

	initSDL();
	createWindow(app);

	std::srand(std::time(nullptr));

	auto storagePath = argv[1];
	Storage storage(storagePath);

	app.assets = Assets(&storage);

	initializeGraphicsAPI(app);

	video::VideoManager videoManager(&app.assets);

	app.assets.Preload<meta::UnitTable>("arr/units.dat");

	app.assets.Preload<meta::SfxTable>("arr/sfxdata.dat");
	app.assets.Preload<data::StringsTable>("arr/sfxdata.tbl");
	
	app.assets.Preload<meta::PortraitTable>("arr/portdata.dat");
	app.assets.Preload<data::StringsTable>("arr/portdata.tbl");

	renderer::DrawableHandle currentFrame = nullptr;
		
	SDL_Event event;

	int unitId = 0;

	app.audioManager = audio::AudioManager(&app.assets);
	app.audioManager.Initialize();

	{
		view::UnitTransmission unitTransmission(&app.assets, &videoManager, app.graphics.get(), &app.audioManager, 60 * 3, 56 * 3);

		unitTransmission.SetUnit(unitId);
		unitTransmission.Fidget();

		Loop loop("Main Loop");
		loop.Start();

		while(loop.IsRunning())
		{
			Cycle cycle = loop.GetNewCycle();

			while (SDL_PollEvent(&event)) {
				switch (event.type) {

					case SDL_EVENT_QUIT:
						loop.Stop();
						break;

					case SDL_EVENT_KEY_DOWN:
						switch(event.key.key)
						{
							case SDLK_LEFT:  unitTransmission.SetUnit(--unitId); unitTransmission.Fidget(); break;
							case SDLK_RIGHT: unitTransmission.SetUnit(++unitId); unitTransmission.Fidget(); break;
							case SDLK_Q: unitTransmission.StartTalk(view::TalkWhat); break;
							case SDLK_W: unitTransmission.StartTalk(view::TalkYes); break;
							case SDLK_E: unitTransmission.StartTalk(view::TalkPissed); break;
							case SDLK_P:
								ShowClockReports();
								break;
						}
						break;
				}
			};

			app.graphics->SetView({0, 0});
			app.graphics->BeginRendering(); // to be in sync with LoadImage()

			unitTransmission.Process(loop.GetDeltaTime());
			unitTransmission.Draw({ 24, 37 });

			app.graphics->PresentToScreen();

			app.audioManager.Process();

			loop.Complete(cycle);
		}
		
		app.graphics->WaitIdle();
	}

	app.graphics->Release();

	app.audioManager.Release();

	freeWindow(app);

	return 0;
}