#include "Command.hpp"
#include <stdexcept>
#include <vulkan/vulkan_core.h>

namespace renderer::vulkan
{
	VkCommandPool CreateCommandPool(Device& device, uint32_t queueFamily, const VkAllocationCallbacks* allocator)
	{
		VkCommandPoolCreateInfo info { VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };

		info.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		info.queueFamilyIndex = queueFamily;

		VkCommandPool commandPool;

		if (vkCreateCommandPool(device, &info, allocator, &commandPool) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create command pool");
		}

		return commandPool;
	}
}
