#pragma once

#include <cstdint>
#include <glm/vec4.hpp>
#include <unordered_map>

#include <data/Common.hpp>
#include <data/Tileset.hpp>
#include <filesystem/Storage.hpp>

#include <data/Palette.hpp>

namespace data
{
	extern std::unordered_map<Tileset, const char*> tileSetNameMap;

	const int TILE_SIZE = 32;

	const int TILE_SQUARE = TILE_SIZE * TILE_SIZE;

	const int CHIP_SIZE = 8;

	const int CHIP_ARRAY_SIZE = 4;

	const int CHIP_PIXELS_AMOUNT = 64;

	enum class TerrainGroupFlags : uint16_t { 
		None = 0, 
		Walkable = 0x0001, Unknown1 = 0x0002, Unwalkable = 0x0004, Unknown2 = 0x0008,
		DoodadCover = 0x0010, Unknown3 = 0x0020, Creep = 0x0040, Unbuildable = 0x0080,
		BlocksView = 0x0100, MidGround = 0x0200, HighGround = 0x0400, Occupied = 0x0800,
		RecedingCreep = 0x1000, Cliff = 0x2000, TempCreep = 0x4000, AllowBeacons = 0x8000
	};

	enum class DoodadGroupFlags : uint16_t {
		None = 0,
		SpriteOverlay = 0x1000,
		UnitOverlay = 0x2000,
		Unused = 0x4000
	};

	enum GroupTypeFlags : uint16_t {
		Unplacable = 0x0, Doodad = 0x1, Terrain = 0xFFFE
	};

	typedef glm::vec<4, uint16_t> Edges;

	union ChipPixels
	{
		uint8_t array[CHIP_SIZE][CHIP_SIZE];
		uint8_t data[CHIP_PIXELS_AMOUNT];
	};

	struct Chip
	{
		ChipPixels palPixels;
	};

	// extended (sc:r) mega tile
	struct Tile
	{
		uint32_t chips[CHIP_ARRAY_SIZE][CHIP_ARRAY_SIZE];

		uint32_t GetChipId(int row, int column) const;
		bool		 IsTileMirrored(int row, int column) const;
	};

	struct TerrainGroup
	{
		GroupTypeFlags 		type;
		TerrainGroupFlags flags;

		Edges 		 edgeTypes;
		Edges 		 terrainPieceType;
		tileID variations[16];
	};

	struct DoodadGroup
	{
		GroupTypeFlags   type;
		DoodadGroupFlags flags;

		uint16_t overlayID;
		uint16_t remasteredDoodad;
		uint16_t groupString;
		uint16_t unused1;
		uint16_t doodadID;
		uint16_t width, height;
		uint16_t unused2;
		tileID tiles[16];

		bool HasFlag(DoodadGroupFlags flag);
	};

	union TileGroup
	{
		GroupTypeFlags type;
		TerrainGroup   terrain;
		DoodadGroup    doodad;
	};

	struct TilesetData : A_TilesetData
	{
		Palette palette;

		data::Tileset tileset;

		int chipCount;
		std::shared_ptr<Chip[]> chips;

		int tilesCount;
		std::shared_ptr<Tile[]> tiles;

		int tileGroupCount;
		std::shared_ptr<TileGroup[]> tileGroups;

		int GetTileCount() const override { return tileGroupCount; }

		int GetTileSize() const override { return TILE_SIZE; };

		void GetPixelData(const tileID tileID, uint8_t* array) const override;
	};

	extern void LoadTilesetData(filesystem::Storage& storage, data::Tileset tileset, TilesetData& out);
}