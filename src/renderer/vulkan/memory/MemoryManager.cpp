#include "MemoryManager.hpp"

#include <algorithm>
#include <cassert>
#include <stdexcept>

namespace renderer::vulkan
{
	using std::runtime_error;

	MemoryManager::MemoryManager() {}

	MemoryManager::MemoryManager(Device* device, const VkAllocationCallbacks* allocator)
		: _device(device), _allocator(allocator) {}

	void MemoryManager::BindMemoryToBuffer(Buffer& buffer, uint32_t typeBits, VkMemoryPropertyFlagBits properties, VkDeviceSize requiredSize)
	{
		auto& memory = UseMemory(typeBits, properties, requiredSize);

		auto [offset, size] = memory.GetLockedMemory(requiredSize);
		assert(size == requiredSize);

		buffer.BindMemory(memory.hwMemory, offset);
	}

	Memory& MemoryManager::UseMemory(uint32_t typeBits, VkMemoryPropertyFlagBits properties, VkDeviceSize requiredSize)
	{
		// Find if memory with this specific properties already exists
		auto memoryIt = std::find_if(_memories.begin(), _memories.end(), 
			[typeBits, properties, requiredSize] (auto& mem) { 
				return mem.typeBits == typeBits && 
							mem.properties == properties && 
							mem.CanAllocateMemory(requiredSize); 
				});

		if (memoryIt != _memories.end())
		{
			return *memoryIt;
		}

		auto typeIndex = FindMemoryType(typeBits, properties);

		VkMemoryAllocateInfo allocInfo { 
			.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO, 
			.pNext = nullptr, 
			.allocationSize = MinimalMemorySize, 
			.memoryTypeIndex = typeIndex };

		VkDeviceMemory memory;

		if (vkAllocateMemory(*_device, &allocInfo, _allocator, &memory) != VK_SUCCESS)
		{
			throw runtime_error("Failed to allocate memory");
		}

		std::vector<MemoryRegion> memoryRegions = { { 0, allocInfo.allocationSize } };

		_memories.emplace_back(typeBits, properties, memory, memoryRegions);

		return _memories.back();
	}

	uint32_t MemoryManager::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
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

	void MemoryManager::Release()
	{
		for(auto memory : _memories)
		{
			vkFreeMemory(*_device, memory.hwMemory, _allocator);
		}
	}

	bool Memory::CanAllocateMemory(VkDeviceSize size)
	{
		return std::any_of(freeRegions.begin(), freeRegions.end(), [size] (auto region) {

			auto [_, regionSize] = region;

			return size <= regionSize;
		});
	}

	MemoryRegion Memory::GetLockedMemory(VkDeviceSize allocatedSize)
	{
		auto it = std::find_if(freeRegions.begin(), freeRegions.end(), [allocatedSize] (auto region) {

			auto [_, regionSize] = region;

			return allocatedSize <= regionSize;

		});

		if (it == freeRegions.end())
		{
			throw runtime_error("Failed to find any memory for buffer");
		}

		auto [offset, space] = *it;
		auto remainingSpace = space - allocatedSize;

		freeRegions.erase(it);

		if (remainingSpace > 0)
			freeRegions.push_back( { offset + allocatedSize, remainingSpace } );

		return { offset, allocatedSize };
	}
};