#include "Device.hpp"

#include "Queue.hpp"
#include "SwapChain.hpp"

#include <algorithm>
#include <array>
#include <cstring>
#include <iterator>
#include <set>
#include <stdexcept>
#include <vector>
#include <vulkan/vulkan_core.h>

using std::array;
using std::any_of;
using std::copy_if;
using std::vector;
using std::runtime_error;
using std::back_inserter;
using std::max_element;
using std::set;

namespace renderer::vulkan
{
	const vector<const char*> deviceExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME,
		VK_KHR_MAINTENANCE1_EXTENSION_NAME
	};

	const vector<const char*> validationLayers = {
		"VK_LAYER_KHRONOS_validation"
	};

	#ifdef NDEBUG
		const bool enableValidationLayers = false;
	#else
		const bool enableValidationLayers = true;
	#endif

	Device::Device() {}

	Device::Device(VkDevice logical, VkPhysicalDevice physical, VkQueue presentQueue, const VkAllocationCallbacks* allocator) : 
		_logical(logical), _physical(physical), _presentQueue(presentQueue), _allocator(allocator)
	{
	}

	void Device::Destroy()
	{
		if (_logical)
			vkDestroyDevice(_logical, _allocator);

		_logical = nullptr;
	}

	Device::operator VkDevice&() { return _logical; }
	Device::operator VkPhysicalDevice&() { return _physical; }

	bool IsSuitableDevice(VkPhysicalDevice& device, VkSurfaceKHR surface);
	bool CheckDeviceExtensionsSupported(VkPhysicalDevice device);

	VkPhysicalDevice PickPhysicalDevice(VkInstance instance, VkSurfaceKHR surface, std::function<int(VkPhysicalDevice&)> evaluationFunction)
	{
		uint32_t count;

		if (vkEnumeratePhysicalDevices(instance, &count, nullptr) != VK_SUCCESS)
		{
			throw runtime_error("Failed to enumerate devices");
		}

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

		bool extensionsSupported = CheckDeviceExtensionsSupported(device);
		bool swapChainSupported = false;

		if (extensionsSupported)
		{
			auto swapChainSupport = QuerySwapChainSupport(device, surface);
			swapChainSupported = swapChainSupport.IsComplete();
		}

		return (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU || props.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) &&
				features.geometryShader &&
				features.samplerAnisotropy &&
				extensionsSupported && 
				swapChainSupported &&
				queueFamilyIndices.IsComplete();
	}

	bool CheckDeviceExtensionsSupported(VkPhysicalDevice device)
	{
		uint32_t extensionsCount;
		if (vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionsCount, nullptr) != VK_SUCCESS)
		{
			throw runtime_error("Failed to enumerate device extension properties");
		}

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

	bool CheckValidationLayerSupported(VkPhysicalDevice device)
	{
		uint32_t propsCount;

		vkEnumerateInstanceLayerProperties(&propsCount, nullptr);

		vector<VkLayerProperties> layerProps(propsCount);

		vkEnumerateInstanceLayerProperties(&propsCount, layerProps.data());

		for (const char* requiredLayer : validationLayers)
		{
			bool layerFound = any_of(
				layerProps.begin(), layerProps.end(),
				[requiredLayer] (auto availableProp) {

					return strcmp(requiredLayer, availableProp.layerName) == 0;
				}
			);

			if (!layerFound)
			{
				return false;
			}
		}

		return true;
	}

	Device Device::Create(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, vector<const char*>& enabledLayers, const VkAllocationCallbacks* allocator)
	{
		QueueFamilyIndices familyIndices       = FindQueueFamilies(physicalDevice, surface);
		set<uint32_t>      uniqueQueueFamilies = { familyIndices.graphicsFamily.value(), familyIndices.presentFamily.value() };

		vector<VkDeviceQueueCreateInfo> queueCreateInfoList;
		queueCreateInfoList.reserve(uniqueQueueFamilies.size());

		for(uint32_t familyIndex : uniqueQueueFamilies)
		{
			const VkDeviceQueueCreateFlags queueFlags = 0;
			const array<float, 1> queuePriorities = { 1.0f };

			queueCreateInfoList.emplace_back(
				VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO, nullptr,
			  queueFlags, familyIndex, 
				queuePriorities.size(), queuePriorities.data());
		}

		VkPhysicalDeviceFeatures deviceFeatures {
			.samplerAnisotropy = VK_TRUE,
		};

		const VkDeviceCreateFlags deviceFlags = 0;
		const uint32_t            enabledLayersCount = 0;

		VkDeviceCreateInfo deviceCreateInfo(
			VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO, nullptr,
			deviceFlags,
			queueCreateInfoList.size(), queueCreateInfoList.data(),
			enabledLayers.size(), enabledLayers.data(),
			deviceExtensions.size(), deviceExtensions.data(), &deviceFeatures);

		// TODO: enable validation layers

		VkDevice logicalDevice;

		if (vkCreateDevice(physicalDevice, &deviceCreateInfo, allocator, &logicalDevice) != VK_SUCCESS)
		{
			throw runtime_error("Failed to create logical device");
		}

		VkQueue presentQueue;

		vkGetDeviceQueue(logicalDevice, familyIndices.presentFamily.value(), 0, &presentQueue);

		return Device(logicalDevice, physicalDevice, presentQueue, allocator);
	}
}