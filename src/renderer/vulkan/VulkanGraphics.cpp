#include "VulkanGraphics.hpp"

#include "device/Device.hpp"
#include "Window.hpp"
#include "device/SwapChain.hpp"

#include <glm/vec2.hpp>
#include <stdexcept>
#include <vector>
#include <vulkan/vulkan_core.h>

using glm::vec2;
using std::runtime_error;
using std::vector;

namespace renderer::vulkan
{
	Graphics::Graphics(SDL_Window* window) : _window(window)
	{
		// Check out layers
		vector<const char*> requiredLayers;

		if (enableValidationLayers)
		{
			if (!CheckValidationLayerSupported(_device))
				throw runtime_error("Validation layer is not supported");

			EnableValidationLayers(requiredLayers);
		}

		CreateInstance(requiredLayers);
		CreateWindowSurface(window, _instance, _surface);

		auto physicalDevice = PickPhysicalDevice(_instance, &DeviceEvaluation);

		// Only one main device is being used
		_device = Device::Create(physicalDevice, _surface, requiredLayers);
		_swapchain = Swapchain::Create(_device, _surface, _window);
	}

	void Graphics::CreateInstance(vector<const char*> enabledLayers)
	{
		std::vector<const char*> extensions;

		GetSdlRequiredExtensions(_window, extensions);

		const char* appName = "gauss-app";
		const char* engineName = "gauss-engine";
		const int   appVersion = 0;
		const int   engineVersion = 0;

		VkApplicationInfo appInfo(
			VK_STRUCTURE_TYPE_APPLICATION_INFO, nullptr, 
			appName, appVersion, engineName, engineVersion, VK_API_VERSION_1_0);

		const VkInstanceCreateFlags flags = 0;

		VkInstanceCreateInfo instanceCreateInfo(
			VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO, nullptr,
			flags, &appInfo, 
			enabledLayers.size(), enabledLayers.data(),
			extensions.size(), extensions.data());

		auto errCode = vkCreateInstance(&instanceCreateInfo, nullptr, &_instance);

		if (errCode != VK_SUCCESS)
		{
			switch(errCode)
			{
				case VK_ERROR_LAYER_NOT_PRESENT:
					throw runtime_error("Failed to create Vulkan API instance: cannot enable validation error");
					break;
				default:
					throw runtime_error("Failed to create Vulkan API instance");
					break;
			}
		}
	}

	void Graphics::EnableValidationLayers(std::vector<const char*>& layerList)
	{
		for(auto layerName : validationLayers)
		{
			layerList.emplace_back(layerName);
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

	/*void Graphics::CycleWaterPalette()
	{
		 примерный алгоритм действий
		
		cyclePaletteColor<1, 6>(app.tilesetAtlas);
		cyclePaletteColor<7, 7>(app.tilesetAtlas);
		
	}*/

	Graphics::~Graphics()
	{
		const VkAllocationCallbacks* const allocator = nullptr;

		_swapchain.Destroy();

		_device.Destroy();

		if (_surface)
			vkDestroySurfaceKHR(_instance, _surface, allocator);

		_surface = nullptr;

		if (_instance)
			vkDestroyInstance(_instance, allocator);

		_instance = nullptr;
	}
}