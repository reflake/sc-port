#pragma once

#include <functional>
#include <vulkan/vulkan_core.h>

namespace renderer::vulkan
{
	extern const bool enableValidationLayers;

	extern bool CheckValidationLayerSupported();

	class Device
	{
	public:

		Device();
		Device(VkDevice, VkPhysicalDevice, VkQueue presentQueue, VkQueue graphicsQueue, const VkAllocationCallbacks*);

		VkQueue GetPresentQueue() { return _presentQueue; }
		VkQueue GetGraphicsQueue() { return _graphicsQueue; }

		// implicit conversion to a logical device reference
		operator VkDevice&();

		// implicit conversion to a physical device reference
		operator VkPhysicalDevice&();

		void Destroy();
	
		static Device Create(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, std::vector<const char*>& enabledLayers, const VkAllocationCallbacks* allocator = nullptr);

	private:

		const VkAllocationCallbacks* _allocator;

		VkDevice 				 _logical;
		VkPhysicalDevice _physical;
		VkQueue          _presentQueue, _graphicsQueue;
	};

	extern VkPhysicalDevice PickPhysicalDevice(VkInstance, VkSurfaceKHR, std::function<int(VkPhysicalDevice&)> evaluationFunction);
}