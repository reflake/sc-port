#include "Shader.hpp"
#include "device/Device.hpp"

#include <array>
#include <stdexcept>
#include <vulkan/vulkan_core.h>

namespace renderer::vulkan
{
	ShaderManager::ShaderManager(Device& device, const VkAllocationCallbacks* allocator) : _device(device), _allocator(allocator)
	{
	}

	ShaderModule ShaderManager::CreateShaderModule(const char* src, size_t size)
	{
		const VkShaderModuleCreateFlags flags = 0;
		VkShaderModuleCreateInfo createInfo { VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO, nullptr, flags, size };

		createInfo.pCode = reinterpret_cast<const uint32_t*>(src);

		VkShaderModule hwModule;

		if (vkCreateShaderModule(_device, &createInfo, _allocator, &hwModule) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create shader module");
		}

		ShaderModule shaderModule = hwModule;

		return shaderModule;
	}

	void ShaderManager::Destroy()
	{
		for(auto module : _modules)
		{
			vkDestroyShaderModule(_device, module, _allocator);
		}
	}

	ShaderModule::ShaderModule(VkShaderModule module) : _module(module) {}

	ShaderModule::operator VkShaderModule() const
	{
		return _module;
	}

}