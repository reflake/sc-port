#pragma once

#include <vulkan/vulkan_core.h>

#include <vector>
#include <sys/types.h>

#include "Device.hpp"
#include "../Config.hpp"

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

	class Swapchain
	{
	public:

		Swapchain() {};
		
		void Destroy();

		static Swapchain Create(Device&, VkSurfaceKHR, Config&, VkAllocationCallbacks* allocator = nullptr);

	private:

		Swapchain(Device*, VkSwapchainKHR, std::vector<VkImageView>&& imageViews, const VkAllocationCallbacks* allocator);

	private:

		const VkAllocationCallbacks* _allocator;

		Device*                  _device;
		VkSwapchainKHR           _hwSwapchain;
		std::vector<VkImageView> _imageViews;
	};
}