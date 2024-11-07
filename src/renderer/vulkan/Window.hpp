#pragma once

#include <tuple>
#include <vulkan/vulkan.h>
#include <SDL_video.h>
#include <vector>

namespace renderer::vulkan {

	class Window
	{
	public:

		Window(SDL_Window* window);

		std::tuple<uint32_t, uint32_t> GetExtent() const;

		operator SDL_Window*() const;

	private:

		SDL_Window* _window;
	};

	extern void GetSdlRequiredExtensions(SDL_Window*, std::vector<const char*>& outExtensions);

	extern void CreateWindowSurface(SDL_Window*, VkInstance&, VkSurfaceKHR&);
}