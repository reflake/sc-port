#include "MemoryManager.hpp"
#include "Image.hpp"
#include "data/Common.hpp"

#include <algorithm>
#include <cassert>
#include <stdexcept>

namespace renderer::vulkan
{
	using std::runtime_error;

	MemoryManager::MemoryManager() {}

	MemoryManager::MemoryManager(Device* device, const VkAllocationCallbacks* allocator)
		: _device(device), _allocator(allocator) {}

	void MemoryManager::BindMemoryToBuffer(Buffer& buffer, VkMemoryRequirements& requirements, VkMemoryPropertyFlagBits properties)
	{
		auto [requiredSize, alignment, typeBits] = requirements;

		auto& memory        = UseMemory(alignment, typeBits, properties, requiredSize);
		auto [offset, size] = memory.GetLockedMemory(alignment, requiredSize);

		assert(size == requiredSize);

		buffer.BindMemory(memory.hwMemory, offset);
	}

	void MemoryManager::BindMemoryToImage(Image& image, VkMemoryRequirements& requirements, VkMemoryPropertyFlagBits properties)
	{
		auto [requiredSize, alignment, typeBits] = requirements;

		if (requiredSize > MinimalMemorySize)
		{
			throw runtime_error("Memory size is too high!");
		}

		auto& memory        = UseMemory(alignment, typeBits, properties, requiredSize);
		auto [offset, size] = memory.GetLockedMemory(alignment, requiredSize);

		assert(size == requiredSize);

		image.BindMemory(memory.hwMemory, offset);
	}

	Memory& MemoryManager::UseMemory(VkDeviceSize alignment, uint32_t typeBits, VkMemoryPropertyFlagBits properties, VkDeviceSize requiredSize)
	{
		// Find if memory with this specific properties already exists
		auto memoryIt = std::find_if(_memories.begin(), _memories.end(), 
			[alignment, typeBits, properties, requiredSize] (auto& mem) { 
				return mem.typeBits == typeBits && 
							mem.properties == properties && 
							mem.CanAllocateMemory(alignment, requiredSize); 
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

	bool Memory::CanAllocateMemory(VkDeviceSize alignment, VkDeviceSize requiredSize)
	{
		return std::any_of(freeRegions.begin(), freeRegions.end(), [alignment, requiredSize] (auto region) {

			auto [offset, regionSize] = region;
			auto alignedOffset = data::Aligned(offset, alignment);
			auto localOffset = alignedOffset - offset;

			return localOffset + requiredSize <= regionSize;
		});
	}

	MemoryRegion Memory::GetLockedMemory(VkDeviceSize alignment, VkDeviceSize allocatedSize)
	{
		auto it = std::find_if(freeRegions.begin(), freeRegions.end(), [alignment, allocatedSize] (auto region) {

			auto [offset, regionSize] = region;
			auto alignedOffset = data::Aligned(offset, alignment);
			auto localOffset = alignedOffset - offset;

			return localOffset + allocatedSize <= regionSize;
		});

		if (it == freeRegions.end())
		{
			throw runtime_error("Failed to find any memory for buffer");
		}

		auto [offset, space] = *it;

		freeRegions.erase(it);

		auto alignedOffset = data::Aligned(offset, alignment);
		auto trailingSpace = alignedOffset - offset;

		if (trailingSpace > 0)
			freeRegions.push_back( { offset, trailingSpace });

		auto headingSpace = space - (allocatedSize + trailingSpace);

		if (headingSpace > 0)
			freeRegions.push_back( { alignedOffset + allocatedSize, headingSpace } );

		// Offset must be aligned
		return { alignedOffset, allocatedSize };
	}
};