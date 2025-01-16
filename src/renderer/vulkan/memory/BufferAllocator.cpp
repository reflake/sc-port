#include "BufferAllocator.hpp"
#include "MemoryManager.hpp"
#include <array>
#include <cstring>
#include <stdexcept>
#include <vulkan/vulkan_core.h>

#include <data/Common.hpp>

namespace renderer::vulkan
{
	using std::runtime_error;
	using std::array;
	using data::Aligned;

	BufferAllocator::BufferAllocator() {}

	BufferAllocator::BufferAllocator(Device* device, MemoryManager* memoryManager, const VkAllocationCallbacks* allocator)
		: _device(device), _memoryManager(memoryManager), _allocator(allocator)
		{}

	void BufferAllocator::Initialize()
	{
		const VkDeviceSize dynamicBufferSize = MinimalMemorySize;

		_dynamicBuffer = Buffer::Create(dynamicBufferSize, _device, _allocator);

		BindMemoryToBuffer(_dynamicBuffer);
	}

	const StreamData BufferAllocator::WriteToStreamBuffer(uint64_t size, const void* srcData)
	{
		if (_dynamicBufferOffset + size > _dynamicBuffer.GetSize())
		{
			throw runtime_error("Failed to write to vertex buffer: buffer size exceeded");
		}

		StreamData streamData = { &_dynamicBuffer, _dynamicBufferOffset, size };

		// Write data to streaming buffer
		uint8_t *dstData = reinterpret_cast<uint8_t*>(_dynamicBufferMappedMemory) + streamData.offsetInMemory;

		memcpy(dstData, srcData, size);

		// Move buffer pointer
		const uint64_t alignment = 16;
		_dynamicBufferOffset += Aligned(_dynamicBufferOffset + size, alignment);

		return streamData;
	}

	// Looks for memory to bind for buffer
	void BufferAllocator::BindMemoryToBuffer(Buffer& buffer)
	{
		VkMemoryRequirements requirements;
		buffer.GetMemoryRequirements(requirements);

		// buffer writing might be not performant 'cause of VK_MEMORY_PROPERTY_HOST_COHERENT_BIT flag
		// TODO: optimization of flushing
		const VkMemoryPropertyFlagBits properMemoryTypeFlags = static_cast<VkMemoryPropertyFlagBits>(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		// TODO: find proper memory for the buffer from list of 
		//  already allocated memories, instead of creation a new one each time
		_memoryManager->BindMemoryToBuffer(buffer, requirements.memoryTypeBits, properMemoryTypeFlags, requirements.size);
	}

	void BufferAllocator::OnBeginRendering()
	{
		_dynamicBufferOffset = 0;

		_dynamicBuffer.MapMemory(&_dynamicBufferMappedMemory);
	}

	void BufferAllocator::OnPrepareForPresentation()
	{
		_dynamicBuffer.UnmapMemory();
	}

	void BufferAllocator::Release()
	{
		_dynamicBuffer.Destroy();
	}

	void StreamData::BindToCommandBuffer(VkCommandBuffer commandBuffer)
	{
		const uint32_t bufferCount = 1;
		array<VkBuffer, bufferCount> buffers = { buffer->GetHandle() };
		array<VkDeviceSize, bufferCount> offsets = { offsetInMemory };

		vkCmdBindVertexBuffers(commandBuffer, 0, bufferCount, buffers.data(), offsets.data());
	}
}