#pragma once

#include <vulkan/vulkan_core.h>

namespace renderer::vulkan
{
	extern VkSurfaceFormatKHR PickSwapSurfaceFormat(VkPhysicalDevice device, VkSurfaceKHR surface);
}