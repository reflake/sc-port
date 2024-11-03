#include "SpriteFrameData.hpp"

namespace renderer
{
	SpriteFrameData::SpriteFrameData(
		std::shared_ptr<uint8_t[]> pixelData, 
		glm::vec<2, uint32_t> dimensions, 
		glm::vec<2, uint32_t> offset) :

		_pixelData(pixelData), _dimensions(dimensions), _offset(offset)
	{
	};

	glm::vec<2, uint32_t> SpriteFrameData::GetDimensions() const { return _dimensions; }
	
	glm::vec<2, uint32_t> SpriteFrameData::GetOffset() const { return _offset; }

	const void* SpriteFrameData::GetPixelData() const { return _pixelData.get(); };
}