#pragma once

#include <memory>
#include <vector>
#include <vulkan/vulkan_core.h>

#include "device/Device.hpp"

#include "Config.hpp"

namespace renderer::vulkan
{
	class Shader;

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
	
		ShaderManager(Device&, Config&, const VkAllocationCallbacks*);

		void Destroy();

		const Shader*       CreateShader(const ShaderModule* modules, int count); // actually creates a whole pipeline for vulkan api
		const ShaderModule* CreateShaderModule(ShaderModule::Stage, const char* src, size_t size);

	private:

		const VkAllocationCallbacks* _allocator;
		const Config&                _config;

		Device& _device;

		std::vector<Shader>       _shaders;
		std::vector<ShaderModule> _modules;
	};

	// Consist rendering pipeline
	class Shader
	{

	};
}