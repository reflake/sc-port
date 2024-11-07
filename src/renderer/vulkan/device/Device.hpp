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
		Device(VkDevice, VkPhysicalDevice);
		~Device();

		// implicit conversion to a logical device reference
		operator VkDevice&();

		// implicit conversion to a physical device reference
		operator VkPhysicalDevice&();

		void Destroy(const VkAllocationCallbacks*);

	private:

		VkDevice 				 _logical;
		VkPhysicalDevice _physical;
	};

	extern VkPhysicalDevice PickPhysicalDevice(VkInstance instance, std::function<int(VkPhysicalDevice&)> evaluationFunction);
	
	extern Device CreateDevice(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkAllocationCallbacks* allocator = nullptr);
}