#pragma once

#include <vulkan/vulkan_core.h>

#include <vector>
#include <sys/types.h>

#include "Device.hpp"
#include "Framebuffer.hpp"
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

		VkFormat GetFormat() const;

		static Swapchain Create(Device&, RenderPass*, VkSurfaceKHR, Config&, VkAllocationCallbacks* allocator = nullptr);

	private:

		Swapchain(Device*, VkSwapchainKHR, VkFormat, std::vector<VkImageView>&& imageViews, std::vector<FrameBuffer>&& framebuffers, const VkAllocationCallbacks* = nullptr);

	private:

		const VkAllocationCallbacks* _allocator;

		Device*                  _device;
		VkSwapchainKHR           _hwSwapchain;
		VkFormat                 _currentFormat;
		std::vector<VkImageView> _imageViews;
		std::vector<FrameBuffer> _frameBuffers;
	};
}