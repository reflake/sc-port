#pragma once

#include <cstdint>
#include <glm/detail/qualifier.hpp>

namespace data
{

	class A_SpriteData
	{
	public:

		virtual void ReadPixelData(uint8_t* out) const = 0;

		virtual uint8_t          GetChannels() const = 0;
		virtual glm::vec<2, int> GetDimensions() const = 0;
	};

	class A_SpriteSheetData
	{
	public:

		virtual const A_SpriteData* GetSprites(int* count) const = 0;

		virtual glm::vec<2, int> GetDimensionsLimit() const = 0;
	};
}