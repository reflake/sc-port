#include <algorithm>
#include "SwapChain.hpp"
#include "Queue.hpp"
#include <SDL_vulkan.h>
#include <algorithm>
#include <cstdint>
#include <glm/ext/vector_float2.hpp>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <vector>
#include <vulkan/vulkan_core.h>

using std::array;
using std::clamp;
using std::min;
using std::runtime_error;
using std::vector;

namespace renderer::vulkan
{
	bool SwapChainSupportDetails::IsComplete()
	{
		return !formats.empty() && !presentModes.empty();
	}

	SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface)
	{
		SwapChainSupportDetails details;

		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

		if (formatCount != 0) 
		{
			details.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
		}

		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

		if (presentModeCount != 0)
		{
			details.presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
		}

		return details;
	}

	VkSurfaceFormatKHR PickSwapSurfaceFormat(const vector<VkSurfaceFormatKHR>& formats);

	VkExtent2D DefineSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
	
	VkPresentModeKHR PickSwapPresentMode(const vector<VkPresentModeKHR>& presentModes);
	
	VkSwapchainKHR CreateSwapchain(Device& device, VkSurfaceKHR surface, Window& window, VkAllocationCallbacks* allocator)
	{
		auto [capabilities, formats, presentModes] = QuerySwapChainSupport(device, surface);

		auto extent = DefineSwapExtent(capabilities);
		auto surfaceFormat = PickSwapSurfaceFormat(formats);
		auto presentMode = PickSwapPresentMode(presentModes);

		uint32_t imageCount = capabilities.minImageCount + 1;

		imageCount = min(imageCount, capabilities.maxImageCount);

		const VkSwapchainCreateFlagsKHR flags = 0;
		const int arrayLayers = 1;

		VkSwapchainCreateInfoKHR createInfo(VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR, nullptr,
			flags, surface, imageCount, surfaceFormat.format, surfaceFormat.colorSpace,
			extent, arrayLayers, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);

		QueueFamilyIndices familyIndices = FindQueueFamilies(device, surface);
		array<uint32_t, 2> familyIndicesArray = { familyIndices.graphicsFamily.value(), 
																							familyIndices.presentFamily.value()};

		// If graphics and present family are the same
		//  then it doesn't have to share resources with itself?
		if (familyIndices.graphicsFamily == familyIndices.presentFamily)
		{
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			createInfo.queueFamilyIndexCount = 0;
			createInfo.pQueueFamilyIndices = nullptr;
		}
		else
		{
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = familyIndicesArray.size();
			createInfo.pQueueFamilyIndices = familyIndicesArray.data();
		}

		createInfo.preTransform = capabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE;
		createInfo.oldSwapchain = VK_NULL_HANDLE; // TODO: pass in old swapchain when window recreated

		VkSwapchainKHR result;

		if (vkCreateSwapchainKHR(device, &createInfo, allocator, &result) != VK_SUCCESS)
		{
			throw runtime_error("Failed to create swapchain");
		}

		TODO: Create images
	}

	VkSurfaceFormatKHR PickSwapSurfaceFormat(const vector<VkSurfaceFormatKHR>& availableFormats)
	{
		for(const auto& format : availableFormats)
		{
			if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			{
				return format;
			}
		}

		throw runtime_error("Unsupported surface format");
	}

	VkExtent2D DefineSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, Window& window)
	{
		if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
		{
			return capabilities.currentExtent;
		}

		auto [ width, height ] = window.GetExtent();

		width = clamp(width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		height = clamp(height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

		return { width, height };
	}      

	VkPresentModeKHR PickSwapPresentMode(const vector<VkPresentModeKHR>& availablePresentModes)
	{
		for (const auto& mode : availablePresentModes)
		{
			if (mode == VK_PRESENT_MODE_MAILBOX_KHR)
			{
				return mode;
			}
		}

		std::cout << "VulkanAPI initialization: picking up present mode, falling back to FIFO present mode\n";

		return VK_PRESENT_MODE_FIFO_KHR;
	}
}