#include "Framebuffer.hpp"

#include <array>

#include <stdexcept>
#include <vulkan/vulkan_core.h>

using std::array;
using std::runtime_error;

namespace renderer::vulkan
{
	FrameBuffer::FrameBuffer(Device* device, VkFramebuffer lwFramebuffer, const VkAllocationCallbacks* allocator) 
		: _device(device), _lwFramebuffer(lwFramebuffer), _allocator(allocator) {}

	FrameBuffer FrameBuffer::CreateBuffer(Device* device, RenderPass* renderPass, Config* config, VkImageView imageView, const VkAllocationCallbacks* allocator)
	{
		VkFramebufferCreateInfo info { VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
		info.renderPass = *renderPass;
		info.attachmentCount = 1;
		info.pAttachments = &imageView;
		info.width  = config->GetExtents().width;
		info.height = config->GetExtents().height;
		info.layers = 1;

		VkFramebuffer lwFramebuffer;

		if (vkCreateFramebuffer(*device, &info, allocator, &lwFramebuffer) != VK_SUCCESS)
		{
			throw runtime_error("Failed to create frame buffer");
		}

		return FrameBuffer(device, lwFramebuffer, allocator);
	}

	void FrameBuffer::Destroy()
	{
		vkDestroyFramebuffer(*_device, _lwFramebuffer, _allocator);
	}

	FrameBuffer::operator VkFramebuffer() const
	{
		return _lwFramebuffer;
	}

}