#pragma once

//#include "Atlas.hpp"
//#include "Texture.hpp"
#include "../device/Device.hpp"

#include "Buffer.hpp"
#include "Image.hpp"
#include "MemoryManager.hpp"

#include <vector>
#include <vulkan/vulkan_core.h>

#include "../Drawable.hpp"

namespace renderer::vulkan
{
	struct StreamData
	{
		Buffer*  buffer = nullptr;
		uint64_t offsetInMemory = 0;
		uint64_t size = 0;

		void BindToCommandBuffer(VkCommandBuffer);
	};

	class BufferAllocator
	{
	public:

		BufferAllocator();
		BufferAllocator(Device* device, VkQueue graphicsQueue, VkCommandPool, MemoryManager*, const VkAllocationCallbacks*);

		void Initialize();

		const StreamData WriteToStreamBuffer(uint64_t size, const void* data);

		void WriteToStreamBuffer(StreamData& streamData, uint64_t size, const void* data);

		const Image* CreateTextureImage(const uint8_t* data, uint32_t width, uint32_t height, uint32_t pixelSize);

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

		Buffer   _stagingBuffer;

		VkQueue  _graphicsQueue = nullptr;

		VkCommandPool   _commandPool = nullptr;
		VkCommandBuffer _stagingCommandBuffer = nullptr;

		std::vector<Image*> _images;
	};
}