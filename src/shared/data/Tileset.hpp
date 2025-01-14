#pragma once

#include "Common.hpp"

#include <cstdint>

namespace data
{
	enum class Tileset: uint16_t { 
		Badlands, SpacePlatform, Installation, 
		Ashworld, Jungle, Desert, Arctic,
		Twilight
	};

	extern bool HasTileSetWater(Tileset tileset);

	struct A_TilesetData
	{

		virtual int GetTileCount() const =0;
		virtual int GetTileSize() const = 0;

		virtual void GetPixelData(const tileID tileID, uint8_t* array) const = 0;
	};
}