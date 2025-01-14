#include "Window.hpp"

#include <SDL_error.h>
#include <cstdint>
#include <stdexcept>
#include <SDL_stdinc.h>
#include <SDL_vulkan.h>

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

		SDL_Vulkan_GetDrawableSize(_window, &w, &h);

		return { w, h };
	}

	Window::operator SDL_Window *() const
	{
		return _window;
	}

	void GetSdlRequiredExtensions(SDL_Window* window, std::vector<const char*>& outExtensions)
	{
		uint32_t count;

		SDL_Vulkan_GetInstanceExtensions(window, &count, nullptr);

		outExtensions.resize(count);

		SDL_Vulkan_GetInstanceExtensions(window, &count, outExtensions.data());
	}

	void CreateWindowSurface(SDL_Window* window, VkInstance instance, VkSurfaceKHR& surface)
	{
		if (SDL_Vulkan_CreateSurface(window, instance, &surface) == SDL_FALSE)
		{
			char errMsg[260];

			SDL_GetErrorMsg(errMsg, 260);

			throw runtime_error(std::string("Failed to create window surface: ") + errMsg);
		}
	}

}
