#include "SwapChain.hpp"
#include "Queue.hpp"
#include <cstdint>
#include <stdexcept>
#include <vulkan/vulkan_core.h>

using std::min;
using std::runtime_error;

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
	
	VkSwapchainKHR CreateSwapchain(Device& device, VkSurfaceKHR surface, VkAllocationCallbacks* allocator = nullptr)
	{
		SwapChainSupportDetails supportDetails = QuerySwapChainSupport(device, surface);

		auto surfaceFormat = PickSwapSurfaceFormat(supportDetails.formats);
		auto presentMode = PickSwapPresentMode(supportDetails.presentModes);
		auto extent = DefineSwapExtent(supportDetails.capabilities);

		uint32_t imageCount = supportDetails.capabilities.minImageCount + 1;

		imageCount = min(imageCount, supportDetails.capabilities.maxImageCount);

		const VkSwapchainCreateFlagsKHR flags = 0;

		VkSwapchainCreateInfoKHR createInfo(VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR, nullptr,
			flags, surface, imageCount, surfaceFormat.format, surfaceFormat.colorSpace,
			extent, 1, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);

		QueueFamilyIndices familyIndices = FindQueueFamilies(device, surface);
		std::array<uint32_t, 2> familyIndicesArray = { familyIndices.graphicsFamily.value(), 
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

		VkSwapchainKHR result;

		if (vkCreateSwapchainKHR(device, &createInfo, allocator, &result) != VK_SUCCESS)
		{
			throw runtime_error("Failed to create swapchain");
		}

		TODO: Create images
	}
}