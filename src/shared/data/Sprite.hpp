#pragma once

#include <cstdint>
#include <glm/vec2.hpp>

namespace data
{

	struct SpriteData
	{
		uint8_t          channels;
		glm::vec<2, int> offset;
		glm::vec<2, int> dimensions;
	};

	class A_SpriteSheetData
	{
	public:

		virtual const SpriteData GetSpriteData(int frame) const = 0;
		virtual const int        ReadPixelData(int frame, uint8_t* out) const = 0;

		virtual glm::vec<2, int> GetDimensionsLimit() const = 0;
	};
}