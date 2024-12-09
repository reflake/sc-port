#pragma once

#include <vulkan/vulkan_core.h>

namespace renderer::vulkan
{
	class Config
	{
	public:

		Config(int screenWidth, int screenHeight);

		const VkDynamicState* GetDynamicStates() const;
		const int             GetDynamicStateCount() const;
		const VkViewport      GetViewport() const;
		const VkExtent2D      GetExtents() const;

	private:

		int screenWidth, screenHeight;
	};
}