#include "Device.hpp"

#include "Queue.hpp"
#include "SwapChain.hpp"

#include <algorithm>
#include <cstring>
#include <iterator>
#include <stdexcept>
#include <vector>
#include <vulkan/vulkan_core.h>

using std::any_of;
using std::copy_if;
using std::vector;
using std::runtime_error;
using std::back_inserter;
using std::max_element;

namespace renderer::vulkan
{
	const vector<const char*> deviceExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME,
		VK_KHR_MAINTENANCE1_EXTENSION_NAME
	};

	bool IsSuitableDevice(VkPhysicalDevice& device, VkSurfaceKHR surface);
	bool CheckDeviceExtensionsSupported(VkPhysicalDevice device);

	VkPhysicalDevice PickPhysicalDevice(VkInstance instance, VkSurfaceKHR surface, std::function<int(VkPhysicalDevice&)> evaluationFunction)
	{
		uint32_t count;

		vkEnumeratePhysicalDevices(instance, &count, nullptr);

		vector<VkPhysicalDevice> devices(count);

		vkEnumeratePhysicalDevices(instance, &count, devices.data());

		vector<VkPhysicalDevice> suitableDevices;
		suitableDevices.reserve(count);

		copy_if(devices.begin(), devices.end(), back_inserter(suitableDevices), 

				[surface] (auto& device) { return IsSuitableDevice(device, surface); });

		if (suitableDevices.empty())
		{
			throw runtime_error("Failed to find any suitable GPU");
		}

		auto bestSuitableDevice = *max_element(
			suitableDevices.begin(), suitableDevices.end(), 
			[evaluationFunction] (auto& a, auto& b) {
				return evaluationFunction(a) < evaluationFunction(b);
			});

		return bestSuitableDevice;
	}

	bool IsSuitableDevice(VkPhysicalDevice& device, VkSurfaceKHR surface)
	{
		VkPhysicalDeviceProperties props;
		VkPhysicalDeviceFeatures features;

		vkGetPhysicalDeviceProperties(device, &props);
		vkGetPhysicalDeviceFeatures(device, &features);

		QueueFamilyIndices queueFamilyIndices = FindQueueFamilies(device, surface);

		bool areExtensionsSupported = CheckDeviceExtensionsSupported(device);
		bool swapChainAdequate = false;

		if (areExtensionsSupported)
		{
			auto swapChainSupport = QuerySwapChainSupport(device, surface);
			swapChainAdequate = swapChainSupport.IsComplete();
		}

		return (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU || props.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) &&
				features.geometryShader &&
				features.samplerAnisotropy &&
				areExtensionsSupported && 
				swapChainAdequate &&
				queueFamilyIndices.IsComplete();
	}

	bool CheckDeviceExtensionsSupported(VkPhysicalDevice device)
	{
		uint32_t extensionsCount;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionsCount, nullptr);

		vector<VkExtensionProperties> availableExtensions(extensionsCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionsCount, availableExtensions.data());

		for(const char *requiredExtension : deviceExtensions)
		{
			bool extensionFound = any_of(
				availableExtensions.begin(), availableExtensions.end(),
				[requiredExtension] (auto availableExtension) { return strcmp(requiredExtension, availableExtension.extensionName) == 0; }
			);

			if (!extensionFound)

				return false;
		}

		return true;
	}
}