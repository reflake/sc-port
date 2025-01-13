#pragma once

//#include "Atlas.hpp"
//#include "Texture.hpp"
#include "../device/Device.hpp"

#include "Buffer.hpp"

#include <memory>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace renderer::vulkan
{
	struct BufferPart
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
		BufferAllocator(Device* device, const VkAllocationCallbacks*);

		void Initialize();

		const BufferPart WriteDynamicVertexBuffer(uint64_t size, const void* data);

		// Needs to be reset every frame
		void OnBeginRendering();

		void OnPrepareForPresentation();

		void Release();

		/*CreateTileset
		CreateVertexData

		std::unique_ptr<Texture> CreateTexture(Atlas& atlas);*/

	private:

		VkBuffer       CreateBuffer(VkDeviceSize);
		VkDeviceMemory CreateMemory(uint32_t typeIndex, VkDeviceSize size);

		// Binds buffer to some memory; might create one as well if needed
		void BindMemoryToBuffer(Buffer& buffer);

		uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags);

	private:

		const VkAllocationCallbacks* _allocator;

		Device* _device;

		Buffer   _dynamicBuffer;
		uint64_t _dynamicBufferOffset = 0;
		void*    _dynamicBufferMappedMemory = nullptr;

		std::vector<VkDeviceMemory>  _memories;
	};
}