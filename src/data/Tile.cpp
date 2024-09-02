#include "Tile.hpp"

namespace data
{
	const static uint32_t MEGA_TILE_ID_MASK = 0xFFFFFFFE;

	uint32_t MegaTile::GetTileId(int row, int column) const
	{
		return (tiles[row][column] & MEGA_TILE_ID_MASK) >> 1;
	}

	bool MegaTile::IsTileMirrored(int row, int column) const
	{
		return tiles[row][column] & ~MEGA_TILE_ID_MASK;
	}
}