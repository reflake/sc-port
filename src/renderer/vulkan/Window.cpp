#include "Window.hpp"

#include <cstdint>
#include <stdexcept>
#include <SDL_stdinc.h>
#include <SDL_vulkan.h>

using std::vector;
using std::runtime_error;

namespace renderer::vulkan
{
	void GetSdlRequiredExtensions(SDL_Window* window, std::vector<const char*>& outExtensions)
	{
		uint32_t count;

		SDL_Vulkan_GetInstanceExtensions(window, &count, nullptr);

		outExtensions.resize(count);

		SDL_Vulkan_GetInstanceExtensions(window, &count, outExtensions.data());
	}

	void CreateWindowSurface(SDL_Window* window, VkInstance& instance, VkSurfaceKHR& surface)
	{
		if (SDL_Vulkan_CreateSurface(window, instance, &surface) == SDL_FALSE)
		{
			throw runtime_error("Failed to create window surface");
		}
	}

}
