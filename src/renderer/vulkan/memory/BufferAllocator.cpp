#include "BufferAllocator.hpp"
#include "Buffer.hpp"
#include "Image.hpp"
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

	BufferAllocator::BufferAllocator(Device* device, VkQueue graphicsQueue, VkCommandPool commandPool, MemoryManager* memoryManager, const VkAllocationCallbacks* allocator)
		: _device(device), _graphicsQueue(graphicsQueue), _commandPool(commandPool), _memoryManager(memoryManager), _allocator(allocator)
		{}

	void BufferAllocator::Initialize()
	{
		const VkDeviceSize dynamicBufferSize = MinimalMemorySize;
		const VkDeviceSize stagingBufferSize = MinimalMemorySize / 4;

		_dynamicBuffer = Buffer::Create(dynamicBufferSize, _device, StreamVertexBuffer, _allocator);

		BindMemoryToBuffer(_dynamicBuffer);

		_stagingBuffer = Buffer::Create(stagingBufferSize, _device, StagingBuffer, _allocator);

		BindMemoryToBuffer(_stagingBuffer);

		VkCommandBufferAllocateInfo commandBufferInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
		commandBufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    commandBufferInfo.commandPool = _commandPool;
    commandBufferInfo.commandBufferCount = 1;

		if (vkAllocateCommandBuffers(*_device, &commandBufferInfo, &_stagingCommandBuffer) != VK_SUCCESS)
		{
			throw runtime_error("Failed to allocate command buffer");
		}
	}
		
	const uint64_t bufferAlignment = 16;

	const StreamData BufferAllocator::WriteToStreamBuffer(uint64_t size, const void* srcData)
	{
		if (_dynamicBufferOffset + size > _dynamicBuffer.GetSize())
		{
			throw runtime_error("Failed to write to vertex buffer: buffer size exceeded");
		}

		_dynamicBufferOffset = Aligned(_dynamicBufferOffset, bufferAlignment);

		StreamData streamData = { &_dynamicBuffer, _dynamicBufferOffset, size };

		// Write data to streaming buffer
		uint8_t *dstData = reinterpret_cast<uint8_t*>(_dynamicBufferMappedMemory) + streamData.offsetInMemory;

		memcpy(dstData, srcData, size);

		// Move buffer pointer
		_dynamicBufferOffset += _dynamicBufferOffset + size;

		return streamData;
	}

	void BufferAllocator::WriteToStreamBuffer(StreamData& streamData, uint64_t size, const void* srcData)
	{
		if (_dynamicBufferOffset + size > _dynamicBuffer.GetSize())
		{
			throw runtime_error("Failed to write to vertex buffer: buffer size exceeded");
		}

		if (streamData.buffer == nullptr)
		{
			_dynamicBufferOffset = Aligned(_dynamicBufferOffset, bufferAlignment);

			streamData.buffer = &_dynamicBuffer;
			streamData.offsetInMemory = _dynamicBufferOffset;
		}

		// Write data to streaming buffer
		uint8_t *dstData = reinterpret_cast<uint8_t*>(_dynamicBufferMappedMemory) + _dynamicBufferOffset;

		memcpy(dstData, srcData, size);

		streamData.size += size;

		// Move buffer pointer
		_dynamicBufferOffset += size;
		
		assert(streamData.offsetInMemory + streamData.size == _dynamicBufferOffset);
	}

	const Image* BufferAllocator::CreateTextureImage(const uint8_t* data, uint32_t width, uint32_t height, uint32_t pixelSize)
	{
		// Creating image
		auto format = pixelSize == 1 ? VK_FORMAT_R8_UNORM : VK_FORMAT_R8G8B8A8_UNORM;
		Image* image = new Image(Image::Create(format, VK_IMAGE_TILING_OPTIMAL, width, height, _device, _allocator));

		VkMemoryRequirements requirements;
		image->GetMemoryRequirements(requirements);

		const VkMemoryPropertyFlagBits properMemoryTypeFlags = image->GetMemoryPropertyFlags();

		_memoryManager->BindMemoryToImage(*image, requirements, properMemoryTypeFlags);

		_images.push_back(image);

		// Copy pixel data to staging buffer
		image->TransitionImageLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, _stagingCommandBuffer, _graphicsQueue);

		void* stagingBufferDst;
		_stagingBuffer.MapMemory(&stagingBufferDst, width * height * pixelSize);

		memcpy(stagingBufferDst, data, width * height * pixelSize);

		_stagingBuffer.UnmapMemory();
		_stagingBuffer.CopyTo(*image, _stagingCommandBuffer, _graphicsQueue);

		image->TransitionImageLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, _stagingCommandBuffer, _graphicsQueue);

		return image;
	}

	// Looks for memory to bind for buffer
	void BufferAllocator::BindMemoryToBuffer(Buffer& buffer)
	{
		VkMemoryRequirements requirements;
		buffer.GetMemoryRequirements(requirements);

		const VkMemoryPropertyFlagBits properMemoryTypeFlags = buffer.GetMemoryPropertyFlags();

		// TODO: find proper memory for the buffer from list of 
		//  already allocated memories, instead of creation a new one each time
		_memoryManager->BindMemoryToBuffer(buffer, requirements, properMemoryTypeFlags);
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
		for(auto image : _images)
		{
			image->Destroy();

			delete image;
		}

		_dynamicBuffer.Destroy();
		_stagingBuffer.Destroy();

		if (_stagingCommandBuffer != VK_NULL_HANDLE)
		{
			vkFreeCommandBuffers(*_device, _commandPool, 1, &_stagingCommandBuffer);
		}

		_stagingCommandBuffer = VK_NULL_HANDLE;
	}

	void StreamData::BindToCommandBuffer(VkCommandBuffer commandBuffer)
	{
		const uint32_t bufferCount = 1;
		array<VkBuffer, bufferCount> buffers = { buffer->GetHandle() };
		array<VkDeviceSize, bufferCount> offsets = { offsetInMemory };

		vkCmdBindVertexBuffers(commandBuffer, 0, bufferCount, buffers.data(), offsets.data());
	}
}