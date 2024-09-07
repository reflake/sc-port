#include <boost/format.hpp>
#include <boost/format/format_fwd.hpp>
#include <cassert>
#include <iostream>
#include <memory>
#include <ostream>
#include <stdexcept>
#include <unordered_map>

#include "Map.hpp"

#include "../filesystem/MpqFile.hpp"

using std::string;
using std::runtime_error;

namespace data
{
	const uint16_t MAX_MAP_SIZE = 256;


	MapInfo::MapInfo(bool onlyEditorInfo) : onlyEditorInfo(onlyEditorInfo) {}

	std::pair<tileGroupID, tileVariation> MapInfo::GetTile(int x, int y)
	{
		auto val = terrain[x + y * dimensions.x];

		return { val >> 4, val & 0xF };
	}


	struct ChunkEntry
	{
		char name[4];
		int dataSize;
	};

	enum EntryName { Unknown = -1, TileSet, Dimensions, Terrain_Gameplay, Terrain_Editor, MapType, Version };

	std::unordered_map<string, EntryName> nameMap = {
		{ "ERA ", TileSet },
		{ "DIM ", Dimensions },
		{ "MTXM", Terrain_Gameplay },
		{ "TILE", Terrain_Editor },
		{ "TYPE", MapType },
		{ "VER ", Version },
	};

	bool ReadChunk(filesystem::MpqFile& file, ChunkEntry& chunk, MapInfo& mapInfo)
	{
		int dataSize = chunk.dataSize;

		assert(dataSize >= 0);

		string nameString = string(chunk.name, 4);
		EntryName nameValue = nameMap.find(nameString) != nameMap.end() ? nameMap[nameString] : Unknown;	

		switch (nameValue) {
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

				if (mapInfo.onlyEditorInfo && nameValue == Terrain_Editor || 
				   !mapInfo.onlyEditorInfo && nameValue == Terrain_Gameplay)
				{
					assert(dataSize <= MAX_MAP_SIZE * MAX_MAP_SIZE * sizeof(uint16_t));

					mapInfo.tileCount = dataSize / sizeof(uint16_t);
					mapInfo.terrain = std::make_shared<uint16_t[]>(mapInfo.tileCount);

					file.ReadBinary(mapInfo.terrain.get(), dataSize);
				}
				else 
				{
					file.Skip(dataSize);
				}
				break;

			case MapType:
				assert(dataSize == sizeof(mapInfo.mapType));
				file.Read(mapInfo.mapType);
				break;

			case Version:
				assert(dataSize == sizeof(mapInfo.version));
				file.Read(mapInfo.version);
				break;

			default:
				file.Skip(dataSize);
				return false;
		}

		return true;
	}

	void ReadMap(filesystem::MpqArchive& mapArchive, MapInfo& mapInfo)
	{
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