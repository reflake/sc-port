#include "Shader.hpp"
#include "device/Device.hpp"

#include <array>
#include <stdexcept>
#include <vulkan/vulkan_core.h>

using std::array;
using std::vector;

namespace renderer::vulkan
{
	using ShaderStage = ShaderModule::Stage;

	ShaderManager::ShaderManager(Device& device, Config& config, const VkAllocationCallbacks* allocator) 
		: _device(device), _config(config), _allocator(allocator)
	{
	}

	VkShaderStageFlagBits GetShaderFlags(ShaderStage stage)
	{
		switch(stage)
		{
			case ShaderStage::Vertex:
				return VK_SHADER_STAGE_VERTEX_BIT;
			case ShaderStage::Fragment:
				return VK_SHADER_STAGE_FRAGMENT_BIT;
		}
	}

	const Shader* ShaderManager::CreateShader(const ShaderModule* modules, int count)
	{
		vector<VkPipelineShaderStageCreateInfo> stageCreateInfoList(count);

		for(int i = 0; i < count; i++)
		{
			auto& module = modules[i];
			VkPipelineShaderStageCreateInfo& stageCreateInfo = stageCreateInfoList[i];

			stageCreateInfo.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			stageCreateInfo.stage  = GetShaderFlags(module.GetType());
			stageCreateInfo.pName  = "main";
			stageCreateInfo.module = module;
		}

		VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo = { VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };

		dynamicStateCreateInfo.pDynamicStates	   = _config.GetDynamicStates();
		dynamicStateCreateInfo.dynamicStateCount = _config.GetDynamicStateCount();

		VkPipelineVertexInputStateCreateInfo vertexInputInfo { VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
		// TODO

		const VkPipelineInputAssemblyStateCreateFlags inputAssemblyFlags = 0;
		const VkBool32                                primiteRestartEnable = VK_FALSE;

		VkPipelineInputAssemblyStateCreateInfo inputAssembly { 
			VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO, 
			nullptr, 
			inputAssemblyFlags,
			VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
			primiteRestartEnable
		};

		array<VkViewport, 1> viewports = { _config.GetViewport() };
		array<VkRect2D, 1>  scissors = { { 0, 0, _config.GetExtents() } };

		const VkPipelineViewportStateCreateFlags viewportStateFlags = 0;
		VkPipelineViewportStateCreateInfo viewportStateCreateInfo { 
			VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO, nullptr, viewportStateFlags,
			viewports.size(), viewports.data(),
			scissors.size(), scissors.data()
		};

		const VkPipelineRasterizationStateCreateFlags rasterizerFlags = 0;

		const auto rasterizerDiscardEnabled = VK_FALSE;
		const auto depthClampEnable = VK_FALSE;
		const auto depthBiasEnable = VK_FALSE;

		// I feel like some of these parameters should be controlled by material not by shader
		VkPipelineRasterizationStateCreateInfo rasterizerCreateInfo { 
			VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO, nullptr, rasterizerFlags,
			rasterizerDiscardEnabled, depthClampEnable, VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT,
			VK_FRONT_FACE_CLOCKWISE, depthBiasEnable
		};

		const VkPipelineMultisampleStateCreateFlags multisampleFlags = 0;

		const auto sampleShadingEnable = VK_FALSE;

		VkPipelineMultisampleStateCreateInfo multisampleStateCreateInfo {
			VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO, nullptr, multisampleFlags,
			VK_SAMPLE_COUNT_1_BIT, sampleShadingEnable, 1.0f, nullptr, VK_FALSE, VK_FALSE,
		};


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