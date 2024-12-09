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

	Swapchain::Swapchain(Device* device, VkSwapchainKHR swapchain, vector<VkImageView>&& imageViews, const VkAllocationCallbacks* allocator) : 
		_device(device), _hwSwapchain(swapchain), _imageViews(imageViews), _allocator(allocator)
	{
	}

	VkSurfaceFormatKHR PickSwapSurfaceFormat(const vector<VkSurfaceFormatKHR>& formats);

	VkExtent2D DefineSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, Config& config);
	
	VkPresentModeKHR PickSwapPresentMode(const vector<VkPresentModeKHR>& presentModes);

	void CreateImageViews(Device& device, VkSwapchainKHR swapchain, vector<VkImageView>& out, VkFormat format, VkAllocationCallbacks* allocator);

	Swapchain Swapchain::Create(Device& device, VkSurfaceKHR surface, Config& config, VkAllocationCallbacks* allocator)
	{
		auto [capabilities, formats, presentModes] = QuerySwapChainSupport(device, surface);

		auto extent = DefineSwapExtent(capabilities, config);
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

		VkSwapchainKHR swapchain;

		if (vkCreateSwapchainKHR(device, &createInfo, allocator, &swapchain) != VK_SUCCESS)
		{
			throw runtime_error("Failed to create swapchain");
		}

		vector<VkImageView> imageViews;
		CreateImageViews(device, swapchain, imageViews, surfaceFormat.format, allocator);

		return Swapchain(&device, swapchain, std::move(imageViews), allocator);
	}

	void CreateImageViews(Device& device, VkSwapchainKHR swapchain, vector<VkImageView>& out, VkFormat format, VkAllocationCallbacks* allocator)
	{
		uint32_t imageCount;

		vkGetSwapchainImagesKHR(device, swapchain, &imageCount, nullptr);
		vector<VkImage> images(imageCount);
		vkGetSwapchainImagesKHR(device, swapchain, &imageCount, images.data());

		out.reserve(imageCount);

		for(auto image : images)
		{
			const VkShaderModuleCreateFlags flags = 0;
			const VkComponentMapping        componentMapping(VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
																				VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY);
			const VkImageSubresourceRange   subresourceRange(VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1);

			VkImageViewCreateInfo createInfo(
				VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO, nullptr, flags,
				image, VK_IMAGE_VIEW_TYPE_2D, format, componentMapping, subresourceRange);

			VkImageView imageView;

			if (vkCreateImageView(device, &createInfo, allocator, &imageView) != VK_SUCCESS)
			{
				throw runtime_error("Failed to create swapchain image view");
			}

			out.push_back(imageView);
		}
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

	VkExtent2D DefineSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, Config& config)
	{
		if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
		{
			return capabilities.currentExtent;
		}

		auto [ width, height ] = config.GetExtents();

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

	void Swapchain::Destroy()
	{
		if (!_imageViews.empty())
		{
			for(auto image : _imageViews)
			{
				vkDestroyImageView(*_device, image, _allocator);
			}
		}

		_imageViews.clear();

		// Destroy swapchain destroys images as well
		if (_hwSwapchain != nullptr)
		{
			vkDestroySwapchainKHR(*_device, _hwSwapchain, _allocator);
		}

		_hwSwapchain = nullptr;
	}
}