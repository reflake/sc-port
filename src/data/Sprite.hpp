#pragma once

#include <cstdint>

#include "Common.hpp"
#include "../filesystem/Storage.hpp"

namespace data
{
	const int MAX_SPRITE_AMOUNT = 517;

	struct SpriteTable
	{
		uint16_t imageID[MAX_SPRITE_AMOUNT];
		uint8_t  unknown[MAX_SPRITE_AMOUNT];
		uint8_t  visible[MAX_SPRITE_AMOUNT];
		
		Array<uint8_t, 130, 517> healthBarLength;
		Array<uint8_t, 130, 517> selectionCircleImage;
		Array<uint8_t, 130, 517> selectionCircleVerticalOffset;
	};

	static_assert(sizeof(SpriteTable) == 3232, "SpriteTable size should be equal to 3232");

	extern void ReadSpriteTable(filesystem::Storage& storage, SpriteTable& spriteTable);
}