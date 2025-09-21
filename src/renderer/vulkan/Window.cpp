#include "Window.hpp"

#include <SDL3/SDL_error.h>
#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_vulkan.h>
#include <cstdint>
#include <stdexcept>

using std::vector;
using std::runtime_error;
using std::tuple;

namespace renderer::vulkan
{
	Window::Window(SDL_Window* window) : _window(window)
	{
	}

	tuple<uint32_t, uint32_t> Window::GetExtent() const
	{
		int w, h;
		SDL_GetWindowSizeInPixels(_window, &w, &h);

		return { w, h };
	}

	Window::operator SDL_Window *() const
	{
		return _window;
	}

	void GetSdlRequiredExtensions(SDL_Window* window, std::vector<const char*>& outExtensions)
	{
		uint32_t count;
		auto extensionsArray = SDL_Vulkan_GetInstanceExtensions(&count);

		outExtensions.resize(count);

		for(int i = 0; i < count; i++)
		{
			outExtensions[i] = extensionsArray[i];
		}
	}

	void CreateWindowSurface(SDL_Window* window, VkInstance instance, VkSurfaceKHR& surface)
	{
		if (SDL_Vulkan_CreateSurface(window, instance, nullptr, &surface) == false)
		{
			const char* errMsg = SDL_GetError();

			throw runtime_error(std::string("Failed to create window surface: ") + errMsg);
		}
	}

}
