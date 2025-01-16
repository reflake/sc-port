#include "Buffer.hpp"
#include <stdexcept>
#include <vulkan/vulkan_core.h>

namespace renderer::vulkan
{
	using std::runtime_error;

	Buffer::Buffer() {}

	Buffer::Buffer(Device* device, VkDeviceSize size, VkBuffer buffer, VkDeviceSize alignment, VkMemoryPropertyFlagBits memoryPropertyFlags, const VkAllocationCallbacks* allocator)
		: _device(device), _size(size), _hwBuffer(buffer), _alignment(alignment), _propertyFlags(memoryPropertyFlags), _allocator(allocator)
		{}

	Buffer Buffer::Create(VkDeviceSize bufferSize, Device* device, BufferType type, const VkAllocationCallbacks* allocator)
	{
		VkBufferCreateInfo info { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
		info.size = bufferSize;
		info.usage = GetUsageFlags(type);
		info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VkBuffer hwBuffer;

		if (vkCreateBuffer(*device, &info, allocator, &hwBuffer) != VK_SUCCESS)
		{
			throw runtime_error("Failed to allocate buffer");
		}

		VkMemoryRequirements requirements;
		vkGetBufferMemoryRequirements(*device, hwBuffer, &requirements);

		auto flags = static_cast<VkMemoryPropertyFlagBits>(GetMemoryPropertyFlags(type));

		return Buffer(device, bufferSize, hwBuffer, requirements.alignment, flags, allocator);
	}

	void Buffer::BindMemory(VkDeviceMemory memory, VkDeviceSize offsetInMemory)
	{
		_hwMemory       = memory;
		_offsetInMemory = offsetInMemory;

		vkBindBufferMemory(*_device, _hwBuffer, memory, offsetInMemory);
	}

	void Buffer::MapMemory(void** dst, uint64_t size, VkDeviceSize offset) const
	{
		const VkFlags memoryFlags = 0;

		if (size == BUFFER_WHOLE_SIZE)

			size = _size;

		vkMapMemory(*_device, _hwMemory, offset, size, memoryFlags, dst);
	}

	void Buffer::UnmapMemory() const
	{
		vkUnmapMemory(*_device, _hwMemory);
	}

	void Buffer::CopyTo(Image& image, VkCommandBuffer commandBuffer, VkQueue queue)
	{
		BeginSingleTimeCommand(commandBuffer);
		{
			VkBufferImageCopy copyRegion = {};

			copyRegion.bufferOffset = 0;
			copyRegion.bufferRowLength = 0;
			copyRegion.bufferImageHeight = 0;

			copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			copyRegion.imageSubresource.mipLevel = 0;
			copyRegion.imageSubresource.baseArrayLayer = 0;
			copyRegion.imageSubresource.layerCount = 1;

			copyRegion.imageOffset = { 0, 0, 0 };
			copyRegion.imageExtent = image.GetExtents();

			vkCmdCopyBufferToImage(commandBuffer, _hwBuffer, image.GetHandle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);
		}

		EndSingleTimeCommandAndSubmit(commandBuffer, queue);
	}
	
	void Buffer::CopyTo(Buffer& dstBuffer, VkDeviceSize size, VkCommandBuffer commandBuffer, VkQueue queue)
	{
		BeginSingleTimeCommand(commandBuffer);
		{
			VkBufferCopy copyRegion = {};
			copyRegion.srcOffset = 0;
			copyRegion.dstOffset = 0;
			copyRegion.size = size;

			vkCmdCopyBuffer(commandBuffer, _hwBuffer, dstBuffer._hwBuffer, 1, &copyRegion);
		}

		EndSingleTimeCommandAndSubmit(commandBuffer, queue);
	}

	void Buffer::BeginSingleTimeCommand(VkCommandBuffer commandBuffer)
	{
		VkCommandBufferBeginInfo beginInfo { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, nullptr,
			 VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT };

		vkResetCommandBuffer(commandBuffer, 0);

		vkBeginCommandBuffer(commandBuffer, &beginInfo);
	}

	void Buffer::EndSingleTimeCommandAndSubmit(VkCommandBuffer commandBuffer, VkQueue queue)
	{
		vkEndCommandBuffer(commandBuffer);

		VkSubmitInfo submitInfo { VK_STRUCTURE_TYPE_SUBMIT_INFO };
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(queue);
	}

	VkDeviceSize Buffer::GetMemoryAlignment() const { return _alignment; }

	VkDeviceMemory Buffer::GetMemoryHandle() const { return _hwMemory; }

	VkDeviceSize Buffer::GetSize() const { return _size; }

	VkMemoryPropertyFlagBits Buffer::GetMemoryPropertyFlags() const { return _propertyFlags; }

	int Buffer::GetMemoryPropertyFlags(BufferType type)
	{
		switch(type)
		{
			// VK_MEMORY_PROPERTY_HOST_COHERENT_BIT flag might decrease performance drastically
			// TODO: optimization of flushing required
			case StreamVertexBuffer:
			case StagingBuffer:
				return VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
		}

		throw runtime_error("Unexpected exception");
	}

	int Buffer::GetUsageFlags(BufferType type)
	{
		switch (type) {
			case StreamVertexBuffer:
				return VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
			case StagingBuffer:
				return VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		}

		throw runtime_error("Unexpected exception");
	}

	void Buffer::GetMemoryRequirements(VkMemoryRequirements& output) const
	{
		vkGetBufferMemoryRequirements(*_device, _hwBuffer, &output);
	}

	VkBuffer Buffer::GetHandle() const { return _hwBuffer; }

	void Buffer::Destroy()
	{
		if (_hwBuffer != VK_NULL_HANDLE)
		{
			vkDestroyBuffer(*_device, _hwBuffer, _allocator);
		}

		_hwBuffer = VK_NULL_HANDLE;
	}
}