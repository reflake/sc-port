#include "VulkanGraphics.hpp"

#include "Device.hpp"
#include "Window.hpp"

#include <glm/vec2.hpp>
#include <stdexcept>
#include <vulkan/vulkan_core.h>

using glm::vec2;
using std::runtime_error;

namespace renderer::vulkan
{
	Graphics::Graphics(SDL_Window* window, filesystem::Storage& storage) : 
		_window(window), _storage(storage)
	{
		CreateInstance();
		CreateWindowSurface(window, _instance, _surface);

		auto physicalDevice = PickPhysicalDevice(_instance, &DeviceEvaluation);

		// Only one main device is being used
		_device = CreateLogicalDevice(physicalDevice, _surface);
	}

	void Graphics::CreateInstance()
	{
		std::vector<const char*> extensions;

		GetSdlRequiredExtensions(_window, extensions);

		const char* appName = "gauss-app";
		const char* engineName = "gauss-engine";
		const int appVersion = 0;
		const int engineVersion = 0;

		VkApplicationInfo appInfo(
			VK_STRUCTURE_TYPE_APPLICATION_INFO, nullptr, 
			appName, appVersion, engineName, engineVersion, VK_API_VERSION_1_0);

		const int enabledLayerCount = 0;

		VkInstanceCreateInfo instanceCreateInfo(
			VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO, &appInfo, 
			enabledLayerCount, nullptr,
			extensions.size(), extensions.data());

		// TODO: implementation
		EnableValidationLayers(instanceCreateInfo);

		if (vkCreateInstance(&instanceCreateInfo, nullptr, &_instance) != VK_SUCCESS)
		{
			throw runtime_error("Failed to create Vulkan API instance");
		}
	}

	int DeviceEvaluation(VkPhysicalDevice& device)
	{
		int score = 0;

		VkPhysicalDeviceProperties props;
		vkGetPhysicalDeviceProperties(device, &props);

		if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
		{
			score += 2000;
		}

		score += props.limits.maxImageDimension2D;
		score += props.limits.maxMemoryAllocationCount;

		return score;
	}
	
	void Graphics::LoadGrp(grpID grpID)
	{
	/* Примерный алгоритм действий для чтения Grp спрайтов

	TextStringsTable imageStrings;
	data::ReadTextStringsTable(storage, "arr/images.tbl", imageStrings);
	uint8_t pixels[256 * 256];


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

		app.spriteAtlases[doodad->grpID] = atlas;*/
	}

	void Graphics::DrawGrpFrame(grpID grpID, uint32_t frame, vec2 position)
	{
		/* Примерный алгоритм отрисовки спрайта на экран
		
		auto spriteAtlas = spriteAtlases[grpID];
		auto surface     = spriteAtlas.surfaces[frame];
		auto destRect    = spriteAtlas.rects[frame];

		destRect.x += doodad->pos.x;
		destRect.y += doodad->pos.y;

		SDL_BlitSurface(surface, nullptr, app.screenSurface, &destRect);*/
	}

	void Graphics::CycleWaterPalette()
	{
		/* примерный алгоритм действий
		
		cyclePaletteColor<1, 6>(app.tilesetAtlas);
		cyclePaletteColor<7, 7>(app.tilesetAtlas);
		*/
	}

	Graphics::~Graphics()
	{
		const VkAllocationCallbacks* const allocator = nullptr;

		if (_device)
			vkDestroyDevice(_device, allocator);

		_device = nullptr;

		if (_surface)
			vkDestroySurfaceKHR(_instance, _surface, allocator);

		_surface = nullptr;

		if (_instance)
			vkDestroyInstance(_instance, allocator);

		_instance = nullptr;
	}
}