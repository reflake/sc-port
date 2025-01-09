#pragma once

#include <memory>
#include <vector>
#include <vulkan/vulkan_core.h>

#include "RenderPass.hpp"
#include "device/Device.hpp"

#include "Config.hpp"

namespace renderer::vulkan
{
	struct ShaderCode
	{
		std::unique_ptr<uint8_t[]> code;
		size_t size;
	};

	// Consist rendering pipeline
	class Shader
	{
	public:

		Shader(VkPipeline, VkPipelineLayout);

		void Destroy(const VkDevice device, const VkAllocationCallbacks* allocator);

		const VkPipeline GetPipeline() const;

	private:

		const VkPipeline _pipeline;
		const VkPipelineLayout _pipelineLayout;
	};

	class ShaderModule
	{
	public:

		enum class Stage : uint32_t 
		{
			Fragment, Vertex
		};

		ShaderModule(VkShaderModule, Stage);

		Stage GetType() const;

		operator VkShaderModule() const;

	private:

		VkShaderModule _module;
		const Stage     _type;
	};

	class ShaderManager
	{
	public:
	
		ShaderManager();
		ShaderManager(Device*, Config*, RenderPass* renderPass, const VkAllocationCallbacks* = nullptr);

		void Destroy();

		const Shader*       CreateShader(const ShaderModule** modules, int count, VkFormat swapchainImageFormat); // actually creates a whole pipeline for vulkan api
		const ShaderModule* CreateShaderModule(ShaderModule::Stage, const ShaderCode& );

	private:

		const VkAllocationCallbacks* _allocator;
		const Config*                _config;

		Device*     _device;
		RenderPass* _renderPass;

		std::vector<Shader>       _shaders;
		std::vector<ShaderModule> _modules;
	};
}