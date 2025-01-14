#include "Format.hpp"

#include <stdexcept>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace renderer::vulkan
{
	VkSurfaceFormatKHR PickSwapSurfaceFormat(VkPhysicalDevice device, VkSurfaceKHR surface)
	{
		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

		std::vector<VkSurfaceFormatKHR> availableFormats(formatCount);

		if (formatCount != 0) 
		{
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, availableFormats.data());
		}

		for(const auto& format : availableFormats)
		{
			if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			{
				return format;
			}
		}

		throw std::runtime_error("Unable to find suitable surface format");
	}
}