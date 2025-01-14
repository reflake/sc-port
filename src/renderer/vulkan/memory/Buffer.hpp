#pragma once

#include "../device/Device.hpp"

#include <vulkan/vulkan_core.h>

namespace renderer::vulkan
{
	const uint64_t BUFFER_WHOLE_SIZE = 0;

	class Buffer
	{
	public:

		Buffer();
		Buffer(Device* device, VkDeviceSize size, VkBuffer, VkDeviceSize alignment, const VkAllocationCallbacks*);

		static Buffer Create(VkDeviceSize bufferSize, Device*, const VkAllocationCallbacks*);

		void BindMemory(VkDeviceMemory, VkDeviceSize offsetInMemory);
		void MapMemory(void** dst, uint64_t size = BUFFER_WHOLE_SIZE, VkDeviceSize offset = 0) const;
		void UnmapMemory() const;

		VkDeviceSize GetMemoryAlignment() const;
		VkDeviceMemory GetMemoryHandle() const;
		VkDeviceSize GetSize() const;

		// Slow operation
		void GetMemoryRequirements(VkMemoryRequirements&) const;

		VkBuffer GetHandle() const;

		void Destroy();

	private:

		Device*        _device = VK_NULL_HANDLE;
		VkDeviceSize   _size = 0, _alignment = 0, _offsetInMemory = 0;
		VkDeviceMemory _hwMemory = VK_NULL_HANDLE;
		VkBuffer       _hwBuffer = VK_NULL_HANDLE;

		const VkAllocationCallbacks* _allocator;
	};
}