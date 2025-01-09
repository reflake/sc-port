#pragma once

#include <vulkan/vulkan_core.h>

#include "Device.hpp"
#include "../Config.hpp"
#include "../RenderPass.hpp"

namespace renderer::vulkan
{
	class FrameBuffer
	{
	public:

		FrameBuffer(Device*, VkFramebuffer, const VkAllocationCallbacks*);

		static FrameBuffer CreateBuffer(Device*, RenderPass*, Config*, VkImageView, const VkAllocationCallbacks* = nullptr);

		void Destroy();

		operator VkFramebuffer() const;

	private:

		Device* _device;
		
		const VkAllocationCallbacks* _allocator;
		const VkFramebuffer _lwFramebuffer;
	};
}