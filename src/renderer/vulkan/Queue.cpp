#include "Queue.hpp"

#include <vulkan/vulkan_core.h>
#include <vector>

namespace renderer::vulkan
{
	using std::vector;

	bool QueueFamilyIndices::IsComplete()
	{
		return graphicsFamily.has_value() && presentFamily.has_value();
	}

	QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface)
	{
		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

		vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

		QueueFamilyIndices indices;
		int index = 0;

		for(const auto& queueFamily : queueFamilies)
		{
			if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)

				indices.graphicsFamily = index;

			VkBool32 presentSupport = false;

			vkGetPhysicalDeviceSurfaceSupportKHR(device, index, surface, &presentSupport);

			if (presentSupport)
				indices.presentFamily = index;

			index++;
		}

		return indices;
	}
}