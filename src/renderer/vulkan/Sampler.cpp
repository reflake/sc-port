#include "Sampler.hpp"
#include <stdexcept>
#include <vulkan/vulkan_core.h>

namespace renderer::vulkan
{
	using Builder = Sampler::Builder;

	Sampler::Sampler() {}

	Sampler::Sampler(Device* device, VkSampler sampler, const VkAllocationCallbacks* allocator) 
		: _device(device), _hwSampler(sampler), _allocator(allocator)
	{
	}

	void Sampler::Destroy()
	{
		if (_hwSampler != VK_NULL_HANDLE)
		{
			vkDestroySampler(*_device, _hwSampler, _allocator);
		}

		_hwSampler = VK_NULL_HANDLE;
	}

	Builder::Builder(Device& device) 
		: _device(device)
	{
		VkPhysicalDeviceProperties properties{};
		vkGetPhysicalDeviceProperties(device, &properties);

		info.maxAnisotropy = properties.limits.maxSamplerAnisotropy / 2.0f;
	}

	Sampler Builder::Create(const VkAllocationCallbacks* allocator)
	{
		VkSampler sampler;

		if (vkCreateSampler(_device, &info, allocator, &sampler) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create sampler");
		}

		return { &_device, sampler, allocator };
	}

	Builder& Builder::UnnormalizedCoordinates(VkBool32 value)
	{
		info.unnormalizedCoordinates = value;
		return *this;
	}

	Builder& Builder::MipmapMode(VkSamplerMipmapMode mode)
	{
		info.mipmapMode = mode;
		return *this;
	}

	Builder& Builder::AnisotropyEnabled(VkBool32 value)
	{
		info.anisotropyEnable = value;
		return *this;
	}
	
	Builder& Builder::Filtering(VkFilter filtering)
	{
		return Filtering(filtering, filtering);
	}

	Builder& Builder::Filtering(VkFilter minFilter, VkFilter magFilter)
	{
		info.minFilter = minFilter;
		info.magFilter = magFilter;
		return *this;
	}

	Builder& Builder::RepeatMode(VkSamplerAddressMode mode)
	{
		info.addressModeU = mode;
		info.addressModeV = mode;
		info.addressModeW = mode;
		return *this;
	}
}