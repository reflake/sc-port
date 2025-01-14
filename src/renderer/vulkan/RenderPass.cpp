#include "RenderPass.hpp"

#include <stdexcept>
#include <vulkan/vulkan_core.h>

using std::runtime_error;

namespace renderer::vulkan
{
	RenderPass::RenderPass()
		{}

	RenderPass::RenderPass(Device* device, VkRenderPass renderPass, VkAllocationCallbacks* allocator) :
		_device(device), _hwRenderPass(renderPass), _allocator(allocator) {}

	RenderPass RenderPass::Create(Device& device, VkFormat format, VkAllocationCallbacks* allocator)
	{
		// create render pass
		VkAttachmentDescription colorAttachment = {};
		colorAttachment.flags   = 0;
		colorAttachment.format  = format;
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp  = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

		colorAttachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference colorAttachmentRef = {};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass {};
		subpass.flags                = 0;
		subpass.pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments    = &colorAttachmentRef;

		subpass.inputAttachmentCount    = 0;
		subpass.pInputAttachments       = VK_NULL_HANDLE;
		subpass.preserveAttachmentCount = 0;
		subpass.pPreserveAttachments    = VK_NULL_HANDLE;

		VkSubpassDependency dependency {};
		dependency.dependencyFlags = 0;
		dependency.srcSubpass      = VK_SUBPASS_EXTERNAL;
		dependency.srcStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask   = 0;
		dependency.dstSubpass      = 0;
		dependency.dstStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		VkRenderPassCreateInfo renderPassInfo = { VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };
		renderPassInfo.attachmentCount = 1;
		renderPassInfo.pAttachments    = &colorAttachment;
		renderPassInfo.subpassCount    = 1;
		renderPassInfo.pSubpasses      = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies   = &dependency;

		VkRenderPass renderPass;

		if (vkCreateRenderPass(device, &renderPassInfo, allocator, &renderPass) != VK_SUCCESS)
		{
			throw runtime_error("Failed to create render pass");
		}

		return RenderPass(&device, renderPass, allocator);
	}

	RenderPass::operator VkRenderPass&()
	{
		return _hwRenderPass;
	}

	void RenderPass::Destroy()
	{
		vkDestroyRenderPass(*_device, _hwRenderPass, _allocator);
	}
}