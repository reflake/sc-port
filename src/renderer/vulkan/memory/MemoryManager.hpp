#pragma once

#include <vector>
#include <vulkan/vulkan_core.h>

#include "../device/Device.hpp"
#include "Buffer.hpp"

namespace renderer::vulkan
{
	const VkDeviceSize MinimalMemorySize = 1024 * 1000;

	typedef std::pair<VkDeviceSize, VkDeviceSize> MemoryRegion;

	struct Memory
	{
		uint32_t typeBits;
		VkMemoryPropertyFlagBits properties;
		VkDeviceMemory hwMemory;

		std::vector<MemoryRegion> freeRegions;

		MemoryRegion GetLockedMemory(VkDeviceSize size);

		bool CanAllocateMemory(VkDeviceSize size);
	};

	class MemoryManager
	{
	public:

		MemoryManager();
		MemoryManager(Device*, const VkAllocationCallbacks*);

		void BindMemoryToBuffer(Buffer& buffer, uint32_t typeBits, VkMemoryPropertyFlagBits, VkDeviceSize size);
		
		void Release();

	private:
	
		Memory&  UseMemory(uint32_t typeBits, VkMemoryPropertyFlagBits, VkDeviceSize size);
		uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags);

	private:
	
		const VkAllocationCallbacks* _allocator;

		Device* _device;

		std::vector<Memory> _memories;
	};
}