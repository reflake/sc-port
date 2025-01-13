#include "BufferAllocator.hpp"
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

	BufferAllocator::BufferAllocator(Device* device, const VkAllocationCallbacks* allocator)
		: _device(device), _allocator(allocator)
		{}

	void BufferAllocator::Initialize()
	{
		const VkDeviceSize dynamicBufferSize = 1024 * 10;

		_dynamicBuffer = Buffer::Create(dynamicBufferSize, _device, _allocator);

		BindMemoryToBuffer(_dynamicBuffer);
	}

	const BufferPart BufferAllocator::WriteDynamicVertexBuffer(uint64_t size, const void* srcData)
	{
		if (_dynamicBufferOffset + size > _dynamicBuffer.GetSize())
		{
			throw runtime_error("Failed to write to vertex buffer: buffer size exceeded");
		}

		BufferPart buffer = { &_dynamicBuffer, _dynamicBufferOffset, size };

		// Write data to freshly created buffer part
		uint8_t *dstData = reinterpret_cast<uint8_t*>(_dynamicBufferMappedMemory) + buffer.offsetInMemory;

		memcpy(dstData, srcData, size);

		// Move buffer pointer
		const uint64_t alignment = 16;
		_dynamicBufferOffset += Aligned(_dynamicBufferOffset + size, alignment);

		return buffer;
	}

	VkDeviceMemory BufferAllocator::CreateMemory(uint32_t typeIndex, VkDeviceSize size)
	{
		VkMemoryAllocateInfo allocInfo { 
			.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO, 
			.pNext = nullptr, 
			.allocationSize = size, 
			.memoryTypeIndex = typeIndex };

		VkDeviceMemory output;

		if (vkAllocateMemory(*_device, &allocInfo, _allocator, &output) != VK_SUCCESS)
		{
			throw runtime_error("Failed to allocate memory");
		}

		_memories.push_back(output);

		return output;
	}

	// Looks for memory to bind for buffer
	void BufferAllocator::BindMemoryToBuffer(Buffer& buffer)
	{
		VkMemoryRequirements requirements;
		buffer.GetMemoryRequirements(requirements);

		// buffer writing might be not performant 'cause of VK_MEMORY_PROPERTY_HOST_COHERENT_BIT flag
		// TODO: optimization of flushing
		const VkFlags properMemoryTypeFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

		// TODO: find proper memory for the buffer from list of 
		//  already allocated memories, instead of creation a new one each time
		auto typeIndex = FindMemoryType(requirements.memoryTypeBits, properMemoryTypeFlags);;
		auto memory    = CreateMemory(typeIndex, requirements.size);

		buffer.BindMemory(memory, 0);
	}

	uint32_t BufferAllocator::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
	{
		VkPhysicalDeviceMemoryProperties memProps;
		vkGetPhysicalDeviceMemoryProperties(*_device, &memProps);

		for (uint32_t i = 0; i < memProps.memoryTypeCount; i++)
		{
			auto memoryType = memProps.memoryTypes[i];
			bool hasSuitableProps = memoryType.propertyFlags & properties;

			if (typeFilter & (1 << i) && hasSuitableProps)
			{
				return i;
			}
		}

		throw runtime_error("Failed to find suitable memory type");
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

		for(auto memory : _memories)
		{
			vkFreeMemory(*_device, memory, _allocator);
		}
	}

	void BufferPart::BindToCommandBuffer(VkCommandBuffer commandBuffer)
	{
		const uint32_t bufferCount = 1;
		array<VkBuffer, bufferCount> buffers = { buffer->GetHandle() };
		array<VkDeviceSize, bufferCount> offsets = { offsetInMemory };

		vkCmdBindVertexBuffers(commandBuffer, 0, bufferCount, buffers.data(), offsets.data());
	}
}