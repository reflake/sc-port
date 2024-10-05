#pragma once

#include <functional>
#include <vulkan/vulkan_core.h>

namespace renderer::vulkan
{
	extern VkPhysicalDevice PickPhysicalDevice(VkInstance instance, std::function<int(VkPhysicalDevice&)> evaluationFunction);
	
	extern VkDevice CreateLogicalDevice(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkAllocationCallbacks* allocator = nullptr);
}