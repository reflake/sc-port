#pragma once

#include <cstdint>

namespace data
{
	struct Tile
	{
		uint8_t palPixels[64];
	};

	// extended (sc:r) mega tile
	struct MegaTile
	{
		uint32_t tiles[4][4];

		uint32_t GetTileId(int row, int column) const;
		bool		 IsTileMirrored(int row, int column) const;
	};
}