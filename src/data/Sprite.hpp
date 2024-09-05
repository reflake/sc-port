#pragma once

#include <cstdint>

#include "../filesystem/Storage.hpp"

namespace data
{
	const int MAX_SPRITE_AMOUNT = 517;
	const int MAX_SPRITE_UNIT_AMOUNT = 387;

	struct SpriteTable
	{
		uint16_t imageID[MAX_SPRITE_AMOUNT];
		uint8_t  healthBarLength[MAX_SPRITE_UNIT_AMOUNT];
		uint8_t  unknown[MAX_SPRITE_AMOUNT];
		uint8_t  visible[MAX_SPRITE_AMOUNT];
		uint8_t  selectionCircleImage[MAX_SPRITE_UNIT_AMOUNT];
		uint8_t  selectionCircleVerticalOffset[MAX_SPRITE_UNIT_AMOUNT];
	};

	extern void ReadSpriteTable(filesystem::Storage& storage, SpriteTable& spriteTable);
}