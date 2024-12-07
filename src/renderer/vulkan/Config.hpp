#pragma once

#include <vulkan/vulkan_core.h>

namespace renderer::vulkan
{
	class Config
	{
	public:

		const VkDynamicState* GetDynamicStates() const;
		const int             GetDynamicStateCount() const;
		const VkViewport      GetViewport() const;
		const VkExtent2D      GetExtents() const;
	};
}