#pragma once

#include <vulkan/vulkan.h>
#include <SDL_video.h>
#include <string>
#include <vector>

namespace renderer::vulkan {

	extern void GetSdlRequiredExtensions(SDL_Window*, std::vector<const char*>& outExtensions);

	extern void CreateWindowSurface(SDL_Window*, VkInstance&, VkSurfaceKHR&);
}