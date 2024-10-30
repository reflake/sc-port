#pragma once

#include <glm/vec2.hpp>

namespace renderer::vulkan
{
	class Atlas
	{
	public:
	
		glm::vec<2, uint32_t> GetMinXY(int frame) const;
		glm::vec<2, uint32_t> GetMaxXY(int frame) const;
	};
}