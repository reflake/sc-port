#pragma once

#include "device/Device.hpp"
#include <array>
#include <vulkan/vulkan_core.h>

namespace renderer::vulkan
{
	enum DescriptorBindType
	{
		BindSampler
	};

	class DescriptorSetLayout
	{
	public:

		class Builder;

		DescriptorSetLayout();
		DescriptorSetLayout(Device*, VkDescriptorSetLayout, const VkAllocationCallbacks* _allocator = nullptr);

		VkDescriptorSetLayout& GetHandle();

		operator VkDescriptorSetLayout&();

		void Destroy();

	private:

		const VkAllocationCallbacks* _allocator;

		Device*               _device;
		VkDescriptorSetLayout _layout;
	};

	const uint32_t MAX_BINDINGS = 12;

	class DescriptorSetLayout::Builder
	{
	public:

		Builder(Device*);

		DescriptorSetLayout Create(const VkAllocationCallbacks* = nullptr);

		Builder AddBinding(DescriptorBindType, uint32_t* binding = nullptr);
	
	private:

		VkDescriptorType   GetBindingType(DescriptorBindType);
		VkShaderStageFlags GetStageFlags(DescriptorBindType);

	private:

		VkDescriptorSetLayoutCreateInfo _createInfo {
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
			.bindingCount = 0,
		};

		Device* _device;

		std::array<VkDescriptorSetLayoutBinding, MAX_BINDINGS> _bindings;
	};
}