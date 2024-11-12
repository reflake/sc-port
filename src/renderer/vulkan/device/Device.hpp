#pragma once

#include <functional>
#include <vulkan/vulkan_core.h>

namespace renderer::vulkan
{
	extern const bool enableValidationLayers;

	extern bool CheckValidationLayerSupported(VkPhysicalDevice device);

	class Device
	{
	public:

		Device();
		Device(VkDevice, VkPhysicalDevice, const VkAllocationCallbacks*);

		// implicit conversion to a logical device reference
		operator VkDevice&();

		// implicit conversion to a physical device reference
		operator VkPhysicalDevice&();

		void Destroy();
	
		static Device Create(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, const VkAllocationCallbacks* allocator = nullptr);

	private:

		const VkAllocationCallbacks* _allocator;

		VkDevice 				 _logical;
		VkPhysicalDevice _physical;
	};

	extern VkPhysicalDevice PickPhysicalDevice(VkInstance instance, std::function<int(VkPhysicalDevice&)> evaluationFunction);
}