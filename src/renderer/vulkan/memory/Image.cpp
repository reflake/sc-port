#include "Image.hpp"
#include <stdexcept>
#include <vulkan/vulkan_core.h>

namespace renderer::vulkan
{
	using std::runtime_error;

	Image::Image() {}

	Image::Image(Device* device, uint32_t width, uint32_t height, 
								VkImage image, VkFormat format, VkImageLayout imageLayout,
							 VkDeviceSize alignment, VkMemoryPropertyFlagBits memoryPropertyFlags, const VkAllocationCallbacks* allocator)
		: _device(device), _width(width), _height(height), _hwImage(image),
			_format(format), _currentLayout(imageLayout), _alignment(alignment), 
			_propertyFlags(memoryPropertyFlags), _allocator(allocator)
		{}

	Image Image::Create(VkFormat format, VkImageTiling tiling, 
											uint32_t width, uint32_t height,  Device* device, const VkAllocationCallbacks* allocator)
	{
		const VkImageLayout initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		VkImageCreateInfo info { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
		info.extent.width = width;
		info.extent.height = height;
		info.extent.depth = 1;
		info.mipLevels = 1;
		info.arrayLayers = 1;
		info.format = format;
		info.tiling = tiling;
		info.imageType = VK_IMAGE_TYPE_2D;
		info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		info.initialLayout = initialLayout;
		info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		info.samples = VK_SAMPLE_COUNT_1_BIT;

		VkImage hwImage;

		if (vkCreateImage(*device, &info, allocator, &hwImage) != VK_SUCCESS)
		{
			throw runtime_error("Failed to allocate buffer");
		}

		VkMemoryRequirements requirements;
		vkGetImageMemoryRequirements(*device, hwImage, &requirements);

		return Image(device, width, height, hwImage, format, initialLayout,
								 requirements.alignment, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, allocator);
	}

	void Image::BindMemory(VkDeviceMemory memory, VkDeviceSize offsetInMemory)
	{
		_offsetInMemory = offsetInMemory;

		vkBindImageMemory(*_device, _hwImage, memory, offsetInMemory);
		
		// When memory is bound we need to instantiate image view
		VkImageViewCreateInfo viewInfo  {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
		viewInfo.image = _hwImage;
		viewInfo.format = _format;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		if (vkCreateImageView(*_device, &viewInfo, _allocator, &_hwImageView) != VK_SUCCESS) {
        throw runtime_error("Failed to create image view");
    }
	}

	void Image::TransitionImageLayout(VkImageLayout nextLayout, VkCommandBuffer commandBuffer, VkQueue queue)
	{
		BeginSingleTimeCommand(commandBuffer);
		{
			VkImageMemoryBarrier barrier{};
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.oldLayout = _currentLayout;
			barrier.newLayout = nextLayout;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.image = _hwImage;
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			barrier.subresourceRange.baseMipLevel = 0;
			barrier.subresourceRange.levelCount = 1;
			barrier.subresourceRange.baseArrayLayer = 0;
			barrier.subresourceRange.layerCount = 1;
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = 0;

			VkPipelineStageFlags sourceStage, destinationStage;

			if (_currentLayout == VK_IMAGE_LAYOUT_UNDEFINED && nextLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
			{
				barrier.srcAccessMask = 0;
				barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

				sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
				destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			}
			else if (_currentLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && nextLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
			{
				barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

				sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
				destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			}
			else
			{
				throw runtime_error("Unsupported layout transition");
			}

			_currentLayout = nextLayout;

			vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 
														0, VK_NULL_HANDLE,
														0, VK_NULL_HANDLE, 
														1, &barrier);
		}

		EndSingleTimeCommandAndSubmit(commandBuffer, queue);
	}

	void Image::BeginSingleTimeCommand(VkCommandBuffer commandBuffer)
	{
		VkCommandBufferBeginInfo beginInfo { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, nullptr,
			 VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT };

		vkResetCommandBuffer(commandBuffer, 0);

		vkBeginCommandBuffer(commandBuffer, &beginInfo);
	}

	void Image::EndSingleTimeCommandAndSubmit(VkCommandBuffer commandBuffer, VkQueue queue)
	{
		vkEndCommandBuffer(commandBuffer);

		VkSubmitInfo submitInfo { VK_STRUCTURE_TYPE_SUBMIT_INFO };
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(queue);
	}

	VkDeviceSize Image::GetMemoryAlignment() const { return _alignment; }

	VkDeviceSize Image::GetSize() const { return _size; }

	VkMemoryPropertyFlagBits Image::GetMemoryPropertyFlags() const { return _propertyFlags; }

	void Image::GetMemoryRequirements(VkMemoryRequirements& output) const
	{
		vkGetImageMemoryRequirements(*_device, _hwImage, &output);
	}

	VkExtent3D Image::GetExtents()
	{
		return { _width, _height, 1 };
	}

	VkImage Image::GetHandle() const { return _hwImage; }

	VkImageView Image::GetViewHandle() const { return _hwImageView; }

	void Image::Destroy()
	{
		if (_hwImageView != VK_NULL_HANDLE)
		{
			vkDestroyImageView(*_device, _hwImageView, _allocator);
		}

		_hwImageView = VK_NULL_HANDLE;

		if (_hwImage != VK_NULL_HANDLE)
		{
			vkDestroyImage(*_device, _hwImage, _allocator);
		}

		_hwImage = VK_NULL_HANDLE;
	}
}