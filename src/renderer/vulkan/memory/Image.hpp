#pragma once

#include "../device/Device.hpp"

#include <vulkan/vulkan_core.h>

namespace renderer::vulkan
{
	const uint64_t IMAGE_WHOLE_SIZE = 0;

	class Image
	{
	public:

		Image();
		Image(Device*, uint32_t width, uint32_t height, VkDeviceSize size, VkImage, 
					VkFormat, VkImageLayout, VkDeviceSize alignment, 
					VkMemoryPropertyFlagBits, const VkAllocationCallbacks*);

		static Image Create(VkFormat format, VkImageTiling tiling, 
												uint32_t width, uint32_t height,
												Device*, const VkAllocationCallbacks*);

		void BindMemory(VkDeviceMemory memory, VkDeviceSize offsetInMemory);
		void TransitionImageLayout(VkImageLayout nextLayout, VkCommandBuffer, VkQueue);

		VkDeviceSize GetMemoryAlignment() const;
		VkDeviceSize GetSize() const;
		VkMemoryPropertyFlagBits GetMemoryPropertyFlags() const;
		VkExtent3D   GetExtents();

		VkDeviceMemory GetMemoryHandle() const;
		VkDeviceSize   GetMemoryOffset() const;

		// Slow operation
		void GetMemoryRequirements(VkMemoryRequirements&) const;

		VkImage GetHandle() const;
		VkImageView GetViewHandle() const;

		void Destroy();

	private:

		void BeginSingleTimeCommand(VkCommandBuffer);
		void EndSingleTimeCommandAndSubmit(VkCommandBuffer, VkQueue);

		VkFormat       _format;
		uint32_t       _width, _height, _pixelSize;
		Device*        _device = VK_NULL_HANDLE;
		VkDeviceSize   _size = 0, _alignment = 0, _offsetInMemory = 0;
		VkImage        _hwImage = VK_NULL_HANDLE;
		VkImageView    _hwImageView = VK_NULL_HANDLE;

		VkDeviceMemory _memory;
		VkDeviceSize   _memoryOffset;

		VkImageLayout            _currentLayout;
		VkMemoryPropertyFlagBits _propertyFlags;

		const VkAllocationCallbacks* _allocator;
	};
}