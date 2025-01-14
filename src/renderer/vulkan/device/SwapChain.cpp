#include <algorithm>
#include "SwapChain.hpp"
#include "Framebuffer.hpp"
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
		return !presentModes.empty();
	}

	SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface)
	{
		SwapChainSupportDetails details;

		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

		if (presentModeCount != 0)
		{
			details.presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
		}

		return details;
	}

	Swapchain::Swapchain(Device* device, VkSwapchainKHR swapchain, VkFormat format, vector<VkImageView>&& imageViews, vector<FrameBuffer>&& frameBuffers, const VkAllocationCallbacks* allocator) : 
		_device(device), _hwSwapchain(swapchain), _currentFormat(format), _imageViews(imageViews), _frameBuffers(frameBuffers), _allocator(allocator)
	{
	}

	VkExtent2D DefineSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, Config& config);
	
	VkPresentModeKHR PickSwapPresentMode(const vector<VkPresentModeKHR>& presentModes);

	void CreateImageViews(Device& device, VkSwapchainKHR swapchain, vector<VkImageView>& out, VkFormat format, VkAllocationCallbacks* allocator);

	void CreateFrameBuffers(Device& device, RenderPass* renderPass, const vector<VkImageView>& imageViews, vector<FrameBuffer>& out, Config& config, VkAllocationCallbacks* allocator);

	Swapchain Swapchain::Create(Device& device, RenderPass* renderPass, VkSurfaceKHR surface, VkSurfaceFormatKHR& surfaceFormat, Config& config, VkAllocationCallbacks* allocator)
	{
		auto [capabilities, presentModes] = QuerySwapChainSupport(device, surface);

		auto extent      = DefineSwapExtent(capabilities, config);
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

		vector<FrameBuffer> framebuffers;
		CreateFrameBuffers(device, renderPass, imageViews, framebuffers, config, allocator);

		return Swapchain(&device, swapchain, surfaceFormat.format, std::move(imageViews), std::move(framebuffers), allocator);
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
	
	void CreateFrameBuffers(Device& device, RenderPass* renderPass, const vector<VkImageView>& imageViews, vector<FrameBuffer>& out, Config& config, VkAllocationCallbacks* allocator)
	{
		for(auto image : imageViews)
		{
			out.push_back(FrameBuffer::CreateBuffer(&device, renderPass, &config, image));
		}
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

	VkFormat Swapchain::GetFormat() const { return _currentFormat; }

	uint32_t Swapchain::GetNextImageIndex(VkSemaphore semaphore) const
	{
		uint32_t imageIndex;

		vkAcquireNextImageKHR(*_device, _hwSwapchain, UINT64_MAX, semaphore, VK_NULL_HANDLE, &imageIndex);

		return imageIndex;
	}

	const FrameBuffer& Swapchain::GetFrameBuffer(uint32_t imageIndex) const
	{
		return _frameBuffers[imageIndex];
	}

	void Swapchain::Destroy()
	{
		if (!_frameBuffers.empty())
		{
			for(auto& buffer : _frameBuffers)
			{
				buffer.Destroy();
			}
		}

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