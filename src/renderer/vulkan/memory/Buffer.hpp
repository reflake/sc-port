#pragma once

#include "../device/Device.hpp"
#include "Image.hpp"

#include <vulkan/vulkan_core.h>

namespace renderer::vulkan
{
	const uint64_t BUFFER_WHOLE_SIZE = 0;

	enum BufferType
	{
		StreamVertexBuffer, StagingBuffer
	};

	class Buffer
	{
	public:

		Buffer();
		Buffer(Device*, VkDeviceSize size, VkBuffer, VkDeviceSize alignment, VkMemoryPropertyFlagBits, const VkAllocationCallbacks*);

		static Buffer Create(VkDeviceSize bufferSize, Device*, BufferType type, const VkAllocationCallbacks*);

		void BindMemory(VkDeviceMemory, VkDeviceSize offsetInMemory);
		void MapMemory(void** dst, uint64_t size = BUFFER_WHOLE_SIZE, VkDeviceSize offset = 0) const;
		void UnmapMemory() const;
		void CopyTo(Image& image, VkCommandBuffer commandBuffer, VkQueue queue);
		void CopyTo(Buffer& dstBuffer, VkDeviceSize size, VkCommandBuffer commandBuffer, VkQueue queue); // only for staging buffers

		VkDeviceSize GetMemoryAlignment() const;
		VkDeviceMemory GetMemoryHandle() const;
		VkDeviceSize GetSize() const;
		VkMemoryPropertyFlagBits GetMemoryPropertyFlags() const;

		// Slow operation
		void GetMemoryRequirements(VkMemoryRequirements&) const;

		VkBuffer GetHandle() const;

		void Destroy();

	private:

		static int GetMemoryPropertyFlags(BufferType);
		static int GetUsageFlags(BufferType);

		void BeginSingleTimeCommand(VkCommandBuffer);
		void EndSingleTimeCommandAndSubmit(VkCommandBuffer, VkQueue);

		Device*        _device = VK_NULL_HANDLE;
		VkDeviceSize   _size = 0, _alignment = 0, _offsetInMemory = 0;
		VkDeviceMemory _hwMemory = VK_NULL_HANDLE;
		VkBuffer       _hwBuffer = VK_NULL_HANDLE;

		VkMemoryPropertyFlagBits _propertyFlags;

		const VkAllocationCallbacks* _allocator;
	};
}