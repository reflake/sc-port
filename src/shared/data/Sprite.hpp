#pragma once

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <glm/vec2.hpp>

namespace data
{

	struct SpriteData
	{
		glm::vec<2, int> offset;
		glm::vec<2, int> dimensions;
	};

	class A_SpriteSheetData
	{
	public:

		virtual const SpriteData GetSpriteData(int frame) const = 0;
		virtual const int        GetSpriteCount() const = 0;
		virtual const int        ReadPixelData(int frame, uint8_t* out, int stride) const = 0;
		virtual const int        GetPixelSize() const = 0;

		virtual glm::vec<2, int> GetDimensionsLimit() const = 0;
	};

	struct SpriteRect
	{
		uint32_t x, y, w, h;
		bool     flipped;
	};

	template<typename T>
	void CopyImage(T* srcImage, uint32_t srcPitch, T* dstImage, SpriteRect dstRect, uint32_t dstPitch)
	{
		uint32_t srcRow = 0;

		for(uint32_t i = 0; i < dstRect.h; i++)
		{
			uint32_t dstRow     = dstRect.y + i;
			void*    pDestRow   = dstImage + dstRow * dstPitch + dstRect.x;
			void*    pSourceRow = srcImage + srcRow * srcPitch;
			uint32_t length     = dstRect.w * sizeof(T);

			memcpy(pDestRow, pSourceRow, length);

			srcRow++;
		}
	};
}