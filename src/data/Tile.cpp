#include "Tile.hpp"
#include "Common.hpp"

namespace data
{
	const static uint32_t MEGA_TILE_ID_MASK = 0xFFFFFFFE;

	std::unordered_map<Tileset, const char*> tileSetNameMap = {
		{ Tileset::Badlands, "badlands" },
		{ Tileset::SpacePlatform, "platform" },
		{ Tileset::Installation, "install" },
		{ Tileset::Ashworld, "ashworld"},
		{ Tileset::Jungle, "jungle" },
		{ Tileset::Desert, "desert" },
		{ Tileset::Arctic, "ice" },
		{ Tileset::Twilight, "twilight" },
	};

	uint32_t MegaTile::GetTileId(int row, int column) const
	{
		return (tiles[row][column] & MEGA_TILE_ID_MASK) >> 1;
	}

	bool MegaTile::IsTileMirrored(int row, int column) const
	{
		return tiles[row][column] & ~MEGA_TILE_ID_MASK;
	}

}