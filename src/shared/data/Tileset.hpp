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

	enum FlipFlags
	{
		FlipNone = 0x0,
		FlipHorizontally = 0x1, FlipVertically = 0x2
	};

	struct A_TilesetData
	{

		virtual int GetTileCount() const = 0;
		virtual int GetTileSize() const  = 0;
		virtual FlipFlags GetFlipFlags(const tileID) const = 0;
		virtual tileID    GetMappedIndex(const tileID) const = 0;

		virtual void GetPixelData(const tileID tileID, uint8_t* dstArray, uint32_t dstOffset, uint32_t dstStride) const = 0;
	};
}