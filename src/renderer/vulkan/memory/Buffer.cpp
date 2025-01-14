#include "Buffer.hpp"
#include <stdexcept>
#include <vulkan/vulkan_core.h>

namespace renderer::vulkan
{
	using std::runtime_error;

	Buffer::Buffer() {}

	Buffer::Buffer(Device* device, VkDeviceSize size, VkBuffer buffer, VkDeviceSize alignment, const VkAllocationCallbacks* allocator)
		: _device(device), _size(size), _hwBuffer(buffer), _alignment(alignment), _allocator(allocator)
		{}

	Buffer Buffer::Create(VkDeviceSize bufferSize, Device* device, const VkAllocationCallbacks* allocator)
	{
		VkBufferCreateInfo info { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
		info.size = bufferSize;
		info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VkBuffer hwBuffer;

		if (vkCreateBuffer(*device, &info, allocator, &hwBuffer) != VK_SUCCESS)
		{
			throw runtime_error("Failed to allocate buffer");
		}

		VkMemoryRequirements requirements;
		vkGetBufferMemoryRequirements(*device, hwBuffer, &requirements);

		return Buffer(device, bufferSize, hwBuffer, requirements.alignment, allocator);
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

	VkDeviceSize Buffer::GetMemoryAlignment() const { return _alignment; }

	VkDeviceMemory Buffer::GetMemoryHandle() const { return _hwMemory; }

	VkDeviceSize Buffer::GetSize() const { return _size; }

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