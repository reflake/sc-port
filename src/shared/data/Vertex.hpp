#pragma once

#include "Common.hpp"
#include <array>
#include <utility>

namespace data
{
	// Returns quad vertices in clockwise order
	/*
   2          3
	  ┌────────┐
		│        │
		│        │
		│        │
   1└────────┘4

	*/
	template<typename V>
	std::array<V, 4> FrameVertices(
		int posLeft, int posTop, int width, int height,
		int texLeft, int texTop, int texWidth, int texHeight,
		FlipFlags flipFlags = FlipNone)
	{
		int posRight  = posLeft + width;
		int posBottom = posTop + height;

		float uvTop    = static_cast<float>(texTop) / texHeight;
		float uvBottom = static_cast<float>(texTop + height) / texHeight;
		float uvLeft   = static_cast<float>(texLeft) / texWidth;
		float uvRight  = static_cast<float>(texLeft + width) / texWidth;

		if (flipFlags & FlipHorizontally)
		{
			std::swap(uvLeft, uvRight);
		}

		if (flipFlags & FlipVertically)
		{
			std::swap(uvTop, uvBottom);
		}

		return {
			V { .pos = { posLeft,  posBottom }, .texCoord = { uvLeft,  uvBottom } },
			V { .pos = { posLeft,  posTop    }, .texCoord = { uvLeft,  uvTop    } },
			V { .pos = { posRight, posTop    }, .texCoord = { uvRight, uvTop    } },
			V { .pos = { posRight, posBottom }, .texCoord = { uvRight, uvBottom } },
		};
	}
}