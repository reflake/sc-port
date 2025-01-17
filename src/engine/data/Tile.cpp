#include <cstring>
#include <data/Common.hpp>
#include <iostream>
#include <memory>
#include <ostream>

#include "Tile.hpp"
#include "data/Tileset.hpp"

using boost::format;

using std::string;
using std::make_shared;

namespace data
{
	const static uint32_t MEGA_TILE_ID_MASK = 0xFFFFFFFE;
	
	const static uint32_t UNIQUE_ID_MIRROR_FLAG = 0x80000000;

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

	FlipFlags TilesetData::GetFlipFlags(const tileID tileID) const
	{
		if (uniqueTileMap[tileID] & UNIQUE_ID_MIRROR_FLAG)
		{
			return FlipHorizontally;
		}

		return FlipNone;
	}

	tileID TilesetData::GetMappedIndex(const tileID tileID) const
	{
		return uniqueTileMap[tileID] & (~UNIQUE_ID_MIRROR_FLAG);
	}

	bool DoodadGroup::HasFlag(DoodadGroupFlags requiredFlag)
	{
		return (static_cast<uint16_t>(flags) & static_cast<uint16_t>(requiredFlag)) > 0;
	}

	void TilesetData::GetPixelData(const tileID tileID, uint8_t* dstArray, uint32_t dstOffset, uint32_t dstStride) const
	{
		auto  tileIndex = uniqueTiles[tileID];
		auto& tile      = tiles[tileIndex];
		
		for(int j = 0; j < TILE_SIZE; j += CHIP_SIZE)
		for(int k = 0; k < TILE_SIZE; k++)
		{
			uint32_t chipId = tile.GetChipId(k / CHIP_SIZE, j / CHIP_SIZE);
			auto&    chip = chips[chipId];

			auto arrayStride = dstArray + dstOffset + j + k * dstStride;

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

		out.palette = Palette(wpeData);

		// Read mini tile's data
		filesystem::StorageFile chipSetFile;
		storage.Open(format("TileSet/%1%.vr4") % tileSetName, chipSetFile);
		
		int chipDataSize = chipSetFile.GetFileSize();
		int chipCount = chipDataSize / sizeof(Tile);
		auto chips = make_shared<Chip[]>(chipCount);

		chipSetFile.ReadBinary(chips.get(), chipDataSize);

		// Read tile's data
		filesystem::StorageFile tileSetFile;
		storage.Open(format("TileSet/%1%.vx4ex") % tileSetName, tileSetFile);

		int tileDataSize = tileSetFile.GetFileSize();
		int tilesCount = tileDataSize / sizeof(Tile);
		auto tiles = make_shared<Tile[]>(tilesCount);

		tileSetFile.ReadBinary(tiles.get(), tileDataSize);

		// Read tile groups
		filesystem::StorageFile tileGroupFile;
		storage.Open(format("TileSet/%1%.cv5") % tileSetName, tileGroupFile);

		int tileGroupDataSize = tileGroupFile.GetFileSize();
		int tileGroupCount = tileGroupDataSize / sizeof(TileGroup);
		auto tileGroups = make_shared<TileGroup[]>(tileGroupCount);

		tileGroupFile.ReadBinary(tileGroups.get(), tileGroupDataSize);

		// Найти похожие тайлы
		auto tileMap     = make_shared<uint32_t[]>(tilesCount);
		auto uniqueTiles = make_shared<uint32_t[]>(tilesCount);
		int  uniqueTilesCount = 0;

		for(int i = 0; i < tilesCount; i++)
		{
			// already mapped
			if (tileMap[i] != 0)
				continue;

			auto&    tile = tiles[i];
			uint32_t mirroredChips[4][4];

			for(int n = 0; n < 4; n++)
			for(int m = 0; m < 4; m++)
			{
				mirroredChips[n][3 - m] = tile.chips[n][m] ^ 0x1;
			};

			tileMap[i] = uniqueTilesCount;

			uniqueTiles[uniqueTilesCount] = i;

			for(int j = i + 1; j < tilesCount; j++)
			{
				auto& otherTile = tiles[j];

				bool same            = memcmp(tile.chips, otherTile.chips, sizeof(tile.chips)) == 0;
				bool sameWhenFlipped = memcmp(mirroredChips, otherTile.chips, sizeof(tile.chips)) == 0;

				if (same)
				{
					tileMap[j] = uniqueTilesCount;
				}
				else if (sameWhenFlipped)
				{
					tileMap[j] = uniqueTilesCount | UNIQUE_ID_MIRROR_FLAG;
				}
			}

			uniqueTilesCount++;
		}

		out.chips     = chips;
		out.chipCount = chipCount;

		out.tileGroups     = tileGroups;
		out.tileGroupCount = tileGroupCount;

		out.tiles     = tiles;
		out.tileCount = tilesCount;

		out.uniqueTiles      = uniqueTiles;
		out.uniqueTilesCount = uniqueTilesCount;

		out.uniqueTileMap = tileMap;

		/*std::cout << "Dup tiles count " << sameTiles << "  avg. distance " << avgDistance << "  max " << maxDistance << std::endl;

		int mirroredTiles = 0;
		maxDistance = 0;
		avgDistance = -1;

		for(int i = 0; i < out.tilesCount; i++)
		{
			auto& tile = out.tiles[i];

			for(int j = i + 1; j < out.tilesCount; j++)
			{
				auto& otherTile = out.tiles[j];

				if (memcmp(otherTile.chips, mirrored, sizeof(tile.chips)) == 0)
				{
					if (maxDistance < (j - i))
					{
						maxDistance = (j - i);
					}

					mirroredTiles++;
					avgDistance = avgDistance < -0.5f ? (j - i) : (avgDistance + j - i) / 2.0;

					break;
				}
			}
		}

		std::cout << "Mirrored tiles count " << mirroredTiles << "  avg. distance " << avgDistance << "  max " << maxDistance << std::endl;

		std::cout << "Total " << out.tilesCount << std::endl;*/
	}
}