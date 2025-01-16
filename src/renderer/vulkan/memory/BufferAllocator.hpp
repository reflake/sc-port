#pragma once

//#include "Atlas.hpp"
//#include "Texture.hpp"
#include "../device/Device.hpp"

#include "Buffer.hpp"
#include "MemoryManager.hpp"

#include <vector>
#include <vulkan/vulkan_core.h>

namespace renderer::vulkan
{
	struct StreamData
	{
		Buffer*  buffer;
		uint64_t offsetInMemory;
		uint64_t size;

		void BindToCommandBuffer(VkCommandBuffer);
	};

	class BufferAllocator
	{
	public:

		BufferAllocator();
		BufferAllocator(Device* device, MemoryManager*, const VkAllocationCallbacks*);

		void Initialize();

		const StreamData WriteToStreamBuffer(uint64_t size, const void* data);

		// Needs to be reset every frame
		void OnBeginRendering();

		void OnPrepareForPresentation();

		void Release();

	private:

		VkBuffer CreateBuffer(VkDeviceSize);

		// Binds buffer to some memory; might create one as well if needed
		void BindMemoryToBuffer(Buffer& buffer);

	private:

		const VkAllocationCallbacks* _allocator;

		MemoryManager* _memoryManager;

		Device* _device;

		Buffer   _dynamicBuffer;
		uint64_t _dynamicBufferOffset = 0;
		void*    _dynamicBufferMappedMemory = nullptr;
	};
}