#include "Shader.hpp"
#include "device/Device.hpp"

#include <array>
#include <stdexcept>
#include <vulkan/vulkan_core.h>

using std::vector;

namespace renderer::vulkan
{
	using ShaderStage = ShaderModule::Stage;

	ShaderManager::ShaderManager(Device& device, const VkAllocationCallbacks* allocator) : _device(device), _allocator(allocator)
	{
	}

	VkShaderStageFlagBits GetNativeStage(ShaderStage stage)
	{
		switch(stage)
		{
			case ShaderStage::Vertex:
				return VK_SHADER_STAGE_VERTEX_BIT;
			case ShaderStage::Fragment:
				return VK_SHADER_STAGE_FRAGMENT_BIT;
		}
	}

	const Shader* CreateShader(const ShaderModule* modules, int count)
	{
		vector<VkPipelineShaderStageCreateInfo> stageCreateInfoList(count);

		for(int i = 0; i < count; i++)
		{
			auto& module = modules[i];
			VkPipelineShaderStageCreateInfo& stageCreateInfo = stageCreateInfoList[i];

			stageCreateInfo.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			stageCreateInfo.stage  = GetNativeStage(module.GetType());
			stageCreateInfo.pName  = "main";
			stageCreateInfo.module = module;
		}

		etc etc
	}

	const ShaderModule* ShaderManager::CreateShaderModule(ShaderModule::Stage type, const char* src, size_t size)
	{
		const VkShaderModuleCreateFlags flags = 0;
		VkShaderModuleCreateInfo createInfo { VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO, nullptr, flags, size };

		createInfo.pCode = reinterpret_cast<const uint32_t*>(src);

		VkShaderModule hwModule;

		if (vkCreateShaderModule(_device, &createInfo, _allocator, &hwModule) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create shader module");
		}

		_modules.emplace_back(hwModule, type);

		return &_modules.back();
	}

	void ShaderManager::Destroy()
	{
		for(auto module : _modules)
		{
			vkDestroyShaderModule(_device, module, _allocator);
		}
	}

	ShaderModule::ShaderModule(VkShaderModule module, Stage type) : _module(module), _type(type) {}

	ShaderModule::operator VkShaderModule() const
	{
		return _module;
	}

	ShaderModule::Stage ShaderModule::GetType() const { return _type; }

}