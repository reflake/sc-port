#include <array>
#include <vulkan/vulkan_core.h>

#include "Config.hpp"

using std::array;

namespace renderer::vulkan
{
	const array<VkDynamicState, 2> _dynamicStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
	
	const VkDynamicState* Config::GetDynamicStates() const
	{
		return _dynamicStates.data();
	}

	const int Config::GetDynamicStateCount() const
	{
		return _dynamicStates.size();
	}

	const VkViewport Config::GetViewport() const
	{
		return {
			0.0f, 0.0f,
			TAKE SWAPCHAIN SIZE,
			0.0, 1.0f
		}
	}

	const VkExtent2D Config::GetExtents() const
	{
		return { GET EXTENTS FROM SURFACE OF WINDOW }
	}
}