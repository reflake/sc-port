#pragma once

#include <memory>
#include <vector>
#include <vulkan/vulkan_core.h>

#include "device/Device.hpp"

namespace renderer::vulkan
{
	class ShaderModule;

	class ShaderManager
	{
	public:
	
		ShaderManager(Device&, const VkAllocationCallbacks*);

		void Destroy();

		ShaderModule CreateShaderModule(const char* src, size_t size);

	private:

		const VkAllocationCallbacks* _allocator;

		Device& _device;

		std::vector<ShaderModule> _modules;
	};

	class ShaderModule
	{
	public:

		ShaderModule(VkShaderModule);

		operator VkShaderModule() const;

	private:

		VkShaderModule _module;
	};
}