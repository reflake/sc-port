#include "Shader.hpp"
#include "device/Device.hpp"

#include <array>
#include <stdexcept>
#include <vulkan/vulkan_core.h>

using std::array;
using std::vector;
using std::runtime_error;

namespace renderer::vulkan
{
	using ShaderStage = ShaderModule::Stage;

	Shader::Shader(VkPipeline pipeline, VkPipelineLayout pipelineLayout)
		: _pipeline(pipeline), _pipelineLayout(pipelineLayout) { }

	ShaderModule::ShaderModule(VkShaderModule module, Stage type) : _module(module), _type(type) { }

	ShaderManager::ShaderManager() {}

	ShaderManager::ShaderManager(Device* device, Config* config, RenderPass* renderPass, const VkAllocationCallbacks* allocator) 
		: _device(device), _config(config), _renderPass(renderPass), _allocator(allocator) { }

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

	const Shader* ShaderManager::CreateShader(const ShaderModule** modules, int count, VkFormat swapchainImageFormat)
	{
		vector<VkPipelineShaderStageCreateInfo> stageCreateInfoList(count);

		for(int i = 0; i < count; i++)
		{
			auto module = modules[i];
			VkPipelineShaderStageCreateInfo& stageCreateInfo = stageCreateInfoList[i];

			stageCreateInfo.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			stageCreateInfo.stage  = GetShaderFlags(module->GetType());
			stageCreateInfo.pName  = "main";
			stageCreateInfo.module = *module;
		}

		VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo = { VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };

		dynamicStateCreateInfo.pDynamicStates	   = _config->GetDynamicStates();
		dynamicStateCreateInfo.dynamicStateCount = _config->GetDynamicStateCount();

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

		array<VkViewport, 1> viewports = { _config->GetViewport() };
		array<VkRect2D, 1>  scissors = { { 0, 0, _config->GetExtents() } };

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

		VkPipelineColorBlendAttachmentState colorBlendAttachmentState { VK_FALSE };

		VkPipelineColorBlendStateCreateInfo colorBlendingCreateInfo { 
			VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO, nullptr, 
			0, VK_FALSE, VK_LOGIC_OP_COPY, 1, &colorBlendAttachmentState, { 0, 0, 0, 0 } };
		
		VkPipelineLayoutCreateInfo pipelineLayoutInfo { 
			VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO, nullptr, 0, 
			0, nullptr, 
			0, nullptr };

		VkPipelineLayout pipelineLayout;

		if (vkCreatePipelineLayout(*_device, &pipelineLayoutInfo, _allocator, &pipelineLayout) != VK_SUCCESS)
		{
			throw runtime_error("Failed to create pipeline layout");
		}

		VkGraphicsPipelineCreateInfo pipelineInfo { VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };
		pipelineInfo.stageCount = count;
		pipelineInfo.pStages = stageCreateInfoList.data();
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportStateCreateInfo;
		pipelineInfo.pRasterizationState = &rasterizerCreateInfo;
		pipelineInfo.pMultisampleState = &multisampleStateCreateInfo;
		pipelineInfo.pDepthStencilState = nullptr;
		pipelineInfo.pColorBlendState = &colorBlendingCreateInfo;
		pipelineInfo.pDynamicState = &dynamicStateCreateInfo;
		pipelineInfo.layout = pipelineLayout;
		pipelineInfo.renderPass = *_renderPass;
		pipelineInfo.subpass = 0;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
		pipelineInfo.basePipelineIndex = -1;

		VkPipeline pipeline;

		if (vkCreateGraphicsPipelines(*_device, VK_NULL_HANDLE, 1, &pipelineInfo, _allocator, &pipeline) != VK_SUCCESS)
		{
			throw runtime_error("Failed to create graphics pipeline");
		}

		return &_shaders.emplace_back(pipeline, pipelineLayout);
	}

	const ShaderModule* ShaderManager::CreateShaderModule(ShaderModule::Stage type, const ShaderCode& shader)
	{
		const VkShaderModuleCreateFlags flags = 0;
		VkShaderModuleCreateInfo createInfo { VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO, nullptr, flags, shader.size };

		createInfo.pCode = reinterpret_cast<const uint32_t*>(shader.code.get());

		VkShaderModule hwModule;

		if (vkCreateShaderModule(*_device, &createInfo, _allocator, &hwModule) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create shader module");
		}

		_modules.emplace_back(hwModule, type);

		return &_modules.back();
	}

	void Shader::Destroy(const VkDevice device, const VkAllocationCallbacks* allocator)
	{
		vkDestroyPipeline(device, _pipeline, allocator);
		vkDestroyPipelineLayout(device, _pipelineLayout, allocator);
	}

	void ShaderManager::Destroy()
	{
		for(auto shader : _shaders)
		{
			shader.Destroy(*_device, _allocator);
		}

		for(auto module : _modules)
		{
			vkDestroyShaderModule(*_device, module, _allocator);
		}
	}

	ShaderModule::operator VkShaderModule() const
	{
		return _module;
	}

	ShaderModule::Stage ShaderModule::GetType() const { return _type; }

}