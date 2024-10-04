#include <algorithm>
#include <boost/format.hpp>
#include <boost/format/format_fwd.hpp>
#include <cassert>
#include <iostream>
#include <memory>
#include <ostream>
#include <stdexcept>
#include <unordered_map>
#include <filesystem/MpqFile.hpp>

#include "Map.hpp"

using std::string;

namespace data
{
	const uint16_t MAX_MAP_SIZE = 256;

	using enum EntryName;

	MapInfo::MapInfo(const std::vector<EntryName>& ignoredEntries) : ignoredEntries(ignoredEntries) {}

	std::pair<tileGroupID, tileVariation> MapInfo::GetTile(int x, int y) const
	{
		auto val = terrain[x + y * dimensions.x];

		return { val >> 4, val & 0xF };
	}

	struct ChunkEntry
	{
		char name[4];
		int dataSize;
	};

	std::unordered_map<string, EntryName> entrySignatureMap = {
		{ "ERA ", TileSet },
		{ "DIM ", Dimensions },
		{ "MTXM", Terrain_Gameplay },
		{ "TILE", Terrain_Editor },
		{ "TYPE", MapType },
		{ "VER ", Version },
		{ "THG2", Sprites_Gameplay}
	};

	bool ReadChunk(filesystem::MpqFile& file, ChunkEntry& chunk, MapInfo& mapInfo)
	{
		int dataSize = chunk.dataSize;

		assert(dataSize >= 0);

		string signature = string(chunk.name, 4);
		EntryName entryName = entrySignatureMap.find(signature) != entrySignatureMap.end() ? entrySignatureMap[signature] : Unknown;
		int count;

		if (std::any_of(mapInfo.ignoredEntries.begin(), 
										mapInfo.ignoredEntries.end(), 
										[entryName](auto& ignoredEntry) {
			return ignoredEntry == entryName;
		}))
		{
			file.Skip(dataSize);
			return true;
		}

		switch (entryName) {
			case TileSet:
				assert(dataSize == sizeof(mapInfo.tileset));
				file.Read(mapInfo.tileset);
				break;

			case Dimensions:
				assert(dataSize == sizeof(mapInfo.dimensions));
				file.Read(mapInfo.dimensions);
				break;

			case Terrain_Gameplay:
			case Terrain_Editor:

				assert(dataSize <= MAX_MAP_SIZE * MAX_MAP_SIZE * sizeof(uint16_t));

				mapInfo.tileCount = dataSize / sizeof(uint16_t);
				mapInfo.terrain = std::make_shared<uint16_t[]>(mapInfo.tileCount);

				file.ReadBinary(mapInfo.terrain.get(), dataSize);
				break;

			case MapType:
				assert(dataSize == sizeof(mapInfo.mapType));
				file.Read(mapInfo.mapType);
				break;

			case Version:
				assert(dataSize == sizeof(mapInfo.version));
				file.Read(mapInfo.version);
				break;

			case Sprites_Gameplay:
				count = dataSize / sizeof(MapSprite);

				mapInfo.sprites.resize(mapInfo.sprites.size() + count);

				file.Read(mapInfo.sprites.data() + mapInfo.sprites.size() - count, count);
				break;

			default:
				file.Skip(dataSize);
				return false;
		}

		return true;
	}

	void Clear(MapInfo& mapInfo)
	{
		mapInfo.sprites.clear();
	}

	void ReadMap(filesystem::MpqArchive& mapArchive, MapInfo& mapInfo)
	{
		Clear(mapInfo);

		filesystem::MpqFile scenarioFile;

		mapArchive.Open("staredit\\scenario.chk", scenarioFile);

		std::vector<string> ignoredEntries;
		int bytesAmount = scenarioFile.GetFileSize();

		while(!scenarioFile.IsEOF())
		{
			ChunkEntry nextEntry;
			scenarioFile.Read(nextEntry);

			if (!ReadChunk(scenarioFile, nextEntry, mapInfo))
			{
				ignoredEntries.push_back(string(nextEntry.name, 4));
			}
		}

		assert(mapInfo.tileCount == mapInfo.dimensions.x * mapInfo.dimensions.y);

		// Remove when done with supporting of all possible entries
		if (ignoredEntries.size() == 0)
			return;

		std::cout << "Map reading: ";

		for(auto& name : ignoredEntries)
		{
			std::cout << boost::format("'%1%' ") % name;
		}

		std::cout << " entries are ignored" << std::endl;
	}
}