#include "DescriptorSetLayout.hpp"

#include <stdexcept>
#include <vulkan/vulkan_core.h>

namespace renderer::vulkan
{
	using std::runtime_error;
	using Builder = DescriptorSetLayout::Builder;

	DescriptorSetLayout::DescriptorSetLayout()
		{}

	DescriptorSetLayout::DescriptorSetLayout(Device* device, VkDescriptorSetLayout layout, const VkAllocationCallbacks* allocator)
		: _device(device), _layout(layout), _allocator(allocator)
	{
	}

	VkDescriptorSetLayout& DescriptorSetLayout::GetHandle()
	{
		return _layout;
	}

	DescriptorSetLayout::operator VkDescriptorSetLayout&()
	{
		return _layout;
	}

	void DescriptorSetLayout::Destroy()
	{
		if (_layout != VK_NULL_HANDLE)
		{
			vkDestroyDescriptorSetLayout(*_device, _layout, _allocator);
		}

		_layout = VK_NULL_HANDLE;
	}

	Builder::Builder(Device* device) 
		: _device(device) {}

	DescriptorSetLayout Builder::Create(const VkAllocationCallbacks* allocator)
	{
		_createInfo.pBindings = _bindings.data();

		VkDescriptorSetLayout layout;

		if (vkCreateDescriptorSetLayout(*_device, &_createInfo, allocator, &layout) != VK_SUCCESS)
		{
			throw runtime_error("Failed to create descriptor set layout");
		}

		return { _device, layout, allocator };
	}

	Builder Builder::AddBinding(DescriptorBindType type, uint32_t* binding)
	{
		uint32_t index = _createInfo.bindingCount++;

		_bindings[index] = VkDescriptorSetLayoutBinding {
			.binding = index,
			.descriptorType = GetBindingType(type),
			.descriptorCount = 1,
			.stageFlags = GetStageFlags(type),
			.pImmutableSamplers = nullptr,
		};

		if (binding != nullptr)
		{
			*binding = _createInfo.bindingCount;
		}

		return *this;
	}

	VkDescriptorType Builder::GetBindingType(DescriptorBindType type)
	{
		switch(type)
		{
			case BindSampler: return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		}

		throw runtime_error("Unknown descriptor type");
	}

	VkShaderStageFlags Builder::GetStageFlags(DescriptorBindType type)
	{
		switch(type)
		{
			case BindSampler: return VK_SHADER_STAGE_FRAGMENT_BIT;
		}

		throw runtime_error("Unknown descriptor type");
	}
}