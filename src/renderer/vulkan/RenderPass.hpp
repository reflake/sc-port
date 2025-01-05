#pragma once

#include "device/Device.hpp"

#include <vulkan/vulkan_core.h>

namespace renderer::vulkan
{
	class RenderPass
	{
	public:

		RenderPass();
		RenderPass(Device*, VkRenderPass, VkAllocationCallbacks*);

		static RenderPass Create(Device& device, VkFormat format, VkAllocationCallbacks* allocator = nullptr);

		void Destroy();

		operator VkRenderPass&();

	private:

		Device* _device;
		VkRenderPass _lwRenderPass;
		VkAllocationCallbacks* _allocator;
	};
}