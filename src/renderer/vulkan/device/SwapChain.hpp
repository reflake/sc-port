#pragma once

#include <vulkan/vulkan_core.h>

#include <vector>
#include <sys/types.h>

#include "Device.hpp"

#include "../Window.hpp"

namespace renderer::vulkan
{
	struct SwapChainSupportDetails
	{
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;

		bool IsComplete();
	};

	extern SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);

	extern VkSwapchainKHR CreateSwapchain(Device& device, VkSurfaceKHR surface, Window& window, VkAllocationCallbacks* allocator = nullptr);
}