#pragma once

#include "device/Device.hpp"
#include <vulkan/vulkan_core.h>

namespace renderer::vulkan
{
	class Sampler
	{
	public:

		class Builder;

		Sampler();
		Sampler(Device*, VkSampler, const VkAllocationCallbacks* = nullptr);

		operator VkSampler() { return _hwSampler; }

		void Destroy();

	private:

		const VkAllocationCallbacks* _allocator;

		Device*   _device;
		VkSampler _hwSampler;
	};

	class Sampler::Builder
	{
	public:

		Builder(Device& device);

		Sampler Create(const VkAllocationCallbacks* = nullptr);

		Sampler::Builder& UnnormalizedCoordinates(VkBool32 value);
		Sampler::Builder& MipmapMode(VkSamplerMipmapMode);
		Sampler::Builder& AnisotropyEnabled(VkBool32 value);
		Sampler::Builder& Filtering(VkFilter filtering);
		Sampler::Builder& Filtering(VkFilter minFilter, VkFilter magFilter);
		Sampler::Builder& RepeatMode(VkSamplerAddressMode mode);

	private:

		VkSamplerCreateInfo info { 
			.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,

			.magFilter = VK_FILTER_LINEAR, 
			.minFilter = VK_FILTER_LINEAR,
			
			.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,

			.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
			.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
			.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,

			.mipLodBias = 1.0f,

			.anisotropyEnable = VK_TRUE,

			.compareEnable = VK_FALSE,
			.compareOp = VK_COMPARE_OP_ALWAYS,

			.minLod = 0.0f,
			.maxLod = 0.0f,

			.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK,

			.unnormalizedCoordinates = VK_FALSE,
		};

		Device& _device;

	};
}