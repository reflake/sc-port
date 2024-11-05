#include <data/Common.hpp>

#include "Tile.hpp"

using boost::format;

using std::string;
using std::make_shared;

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

	uint32_t Tile::GetChipId(int row, int column) const
	{
		return (chips[row][column] & MEGA_TILE_ID_MASK) >> 1;
	}

	bool Tile::IsTileMirrored(int row, int column) const
	{
		return chips[row][column] & ~MEGA_TILE_ID_MASK;
	}

	bool DoodadGroup::HasFlag(DoodadGroupFlags requiredFlag)
	{
		return (static_cast<uint16_t>(flags) & static_cast<uint16_t>(requiredFlag)) > 0;
	}

	void TilesetData::GetPixelData(const tileID tileID, uint8_t* array) const
	{
		auto tile = tiles[tileID];
		
		for(int j = 0; j < TILE_SIZE; j += TILE_SIZE)
		for(int k = 0; k < TILE_SIZE; k++)
		{
			uint32_t chipId = tile.GetChipId(k / CHIP_SIZE, j / CHIP_SIZE);
			auto&    chip = chips[chipId];

			auto arrayStride = array + j + k * TILE_SIZE;

			if (tile.IsTileMirrored(k / CHIP_SIZE, j / CHIP_SIZE))
			{
				for(int n = 0; n < CHIP_SIZE; n++)

					arrayStride[CHIP_SIZE - 1 - n] = chip.palPixels.array[k % CHIP_SIZE][n];
			}
			else
			{
				memcpy(arrayStride, chip.palPixels.array[k % 8], CHIP_SIZE);
			}
		}
	}

	void LoadTilesetData(filesystem::Storage& storage, data::Tileset tileset, TilesetData& out)
	{
		out.tileset = tileset;

		// Loading palette
		string tileSetName = data::tileSetNameMap[tileset];

		data::WpeData wpeData;
		storage.Read(format("TileSet/%1%.wpe") % tileSetName, wpeData);

		out.paletteData = Palette(wpeData);

		// Read mini tile's data
		filesystem::StorageFile chipSetFile;
		storage.Open(format("TileSet/%1%.vr4") % tileSetName, chipSetFile);
		
		int chipDataSize = chipSetFile.GetFileSize();
		
		out.chipCount = chipDataSize / sizeof(Tile);
		out.chips = make_shared<Chip[]>(out.chipCount);
		chipSetFile.ReadBinary(out.chips.get(), chipDataSize);

		// Read mega tile's data
		filesystem::StorageFile tileSetFile;
		storage.Open(format("TileSet/%1%.vx4ex") % tileSetName, tileSetFile);

		int tileDataSize = tileSetFile.GetFileSize();

		out.tilesCount = tileDataSize / sizeof(Tile);
		out.tiles = make_shared<Tile[]>(out.tilesCount);
		tileSetFile.ReadBinary(out.tiles.get(), tileDataSize);

		// Read tile groups
		filesystem::StorageFile tileGroupFile;
		storage.Open(format("TileSet/%1%.cv5") % tileSetName, tileGroupFile);

		int tileGroupDataSize = tileGroupFile.GetFileSize();

		out.tileGroupCount = tileGroupDataSize / sizeof(TileGroup);
		out.tileGroups = make_shared<TileGroup[]>(out.tileGroupCount);
		tileGroupFile.ReadBinary(out.tileGroups.get(), tileGroupDataSize);
	}
}