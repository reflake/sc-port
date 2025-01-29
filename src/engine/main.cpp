#include "A_Graphics.hpp"
#include "data/TextStrings.hpp"
#include "diagnostic/Clock.hpp"
#include "meta/PortraitTable.hpp"
#include "meta/UnitTable.hpp"
#include "view/UnitTransmission.hpp"
#include "vulkan/Api.hpp"
#include <SDL_keycode.h>
#include <SDL_timer.h>
#include <SDL_video.h>
#include <stdexcept>
#include <unistd.h>
#define SDL_MAIN_HANDLED

#include <SDL.h>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <iostream>
#include <memory>
#include <sstream>

#include "data/Assets.hpp"
#include "diagnostic/Image.hpp"
#include "video/Decoder.hpp"
#include "video/Video.hpp"

#include <boost/foreach.hpp>
#include <boost/format/format_fwd.hpp>

#include <filesystem/Storage.hpp>

using boost::format;
using std::runtime_error;

void throwSdlError(const char* msg)
{
	auto err = format("%1%: %2%") % msg % SDL_GetError();

	throw runtime_error( err.str() );
}

struct App
{
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
	const int windowFlags = SDL_WINDOW_SHOWN | SDL_WINDOW_VULKAN;
	const int SCREEN_WIDTH = 1280;
	const int SCREEN_HEIGHT = 960;

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
	app.assets.Preload<meta::PortraitTable>("arr/portdata.dat");
	app.assets.Preload<data::TextStringsTable>("arr/portdata.tbl");

	renderer::DrawableHandle currentFrame = nullptr;
		
	SDL_Event event;

	uint64_t lastCounter = SDL_GetPerformanceCounter();
	double   deltaTime, realTime = 0.0;
	double   nextFrameTime = -1.0;

	bool running = true;

	int unitId = 0;

	view::UnitTransmission unitTransmission(&app.assets, &videoManager, app.graphics.get(), 60 * 3, 56 * 3);

	unitTransmission.SetUnit(unitId);
	unitTransmission.Fidget();

	while(running)
	{
		Clock clock("Loop");

		while (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_QUIT:
				running = false;
				break;
			case SDL_KEYDOWN:
				switch(event.key.keysym.sym)
				{
					case SDLK_LEFT:  unitTransmission.SetUnit(--unitId); unitTransmission.Fidget(); break;
					case SDLK_RIGHT: unitTransmission.SetUnit(++unitId); unitTransmission.Fidget(); break;
					case SDLK_UP: unitTransmission.StartTalk(view::TalkWhat); break;
					case SDLK_p:
						ShowClockReports();
						break;
				}
				break;
			}
		};

		app.graphics->SetView({0, 0});
		app.graphics->BeginRendering(); // to be in sync with LoadImage()

		unitTransmission.Process(deltaTime);

		unitTransmission.Draw({ 24, 24 });

		app.graphics->PresentToScreen();

		usleep(1000); // throttling

		uint64_t currentCounter = SDL_GetPerformanceCounter();

		deltaTime = static_cast<double>(currentCounter - lastCounter) / SDL_GetPerformanceFrequency();
		realTime += deltaTime;

		lastCounter = currentCounter;
	}

	app.graphics->WaitIdle();
	app.graphics->Release();

	freeWindow(app);

	return 0;
}