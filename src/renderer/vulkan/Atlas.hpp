#pragma once

#include "data/Sprite.hpp"
#include <glm/vec2.hpp>
#include <vector>

namespace renderer::vulkan
{
	class Atlas
	{
	public:

		Atlas(uint32_t width, uint32_t height, std::vector<data::SpriteRect>&&);
	
		glm::vec<2, uint32_t> GetDimensions() const;
		data::SpriteRect      GetFrame(int frame) const;

	private:

		glm::vec<2, uint32_t>         _dimensions;
		std::vector<data::SpriteRect> _rectangles;
	};
}