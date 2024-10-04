#pragma once

#include <cstdint>

#include <filesystem/Storage.hpp>

namespace data
{
	const int MAX_IMAGES_AMOUNT = 999;

	enum class PaletteRemap : uint8_t { 
			None = 0, 
			OrangeFire, 
			GreenFire, 
			BlueFire,
			BlueExplosion,
			Special,
	 };

	struct ImagesTable
	{
		uint32_t grpID[MAX_IMAGES_AMOUNT];
		uint8_t  turns[MAX_IMAGES_AMOUNT];
		bool     selectable[MAX_IMAGES_AMOUNT];
		bool     useFullIScript[MAX_IMAGES_AMOUNT];
		bool     drawIfCloaked[MAX_IMAGES_AMOUNT];
		uint8_t  drawFunction[MAX_IMAGES_AMOUNT];
		uint32_t iScriptID[MAX_IMAGES_AMOUNT];
		uint32_t shieldOverlay[MAX_IMAGES_AMOUNT];
		uint32_t attackOverlay[MAX_IMAGES_AMOUNT];
		uint32_t damageOverlay[MAX_IMAGES_AMOUNT];
		uint32_t specialOverlay[MAX_IMAGES_AMOUNT];
		uint32_t landingOverlay[MAX_IMAGES_AMOUNT];
		uint32_t liftOffOverlay[MAX_IMAGES_AMOUNT];

		PaletteRemap remapping[MAX_IMAGES_AMOUNT];
	};

	static_assert(sizeof(ImagesTable) == 37964, "ImagesTable size should be equal to 37964");

	extern void ReadImagesTable(filesystem::Storage& storage, ImagesTable& table);
};