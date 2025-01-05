#pragma once

#include "device/Device.hpp"

#include <vulkan/vulkan_core.h>

namespace renderer::vulkan
{
	VkCommandPool CreateCommandPool(Device& device, uint32_t queueFamily, const VkAllocationCallbacks* allocator = nullptr);
}