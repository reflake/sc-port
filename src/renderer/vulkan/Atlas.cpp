#include "Atlas.hpp"

namespace renderer::vulkan
{
	Atlas::Atlas(uint32_t width, uint32_t height, std::vector<data::SpriteRect>& rectangles) : 
		_dimensions(width, height), 
		_rectangles(std::move(rectangles))
		{}

	glm::vec<2, uint32_t> Atlas::GetDimensions() const
	{
		return _dimensions;
	}

	data::SpriteRect Atlas::GetFrame(int frame) const
	{
		return _rectangles[frame];
	}
};