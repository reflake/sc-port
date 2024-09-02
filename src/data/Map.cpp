#include "Map.hpp"
#include <boost/format.hpp>
#include <boost/format/format_fwd.hpp>
#include <cassert>
#include <iostream>
#include <memory>
#include <ostream>
#include <stdexcept>
#include <unordered_map>

using std::string;
using std::runtime_error;

namespace data
{
	const uint16_t MAX_MAP_SIZE = 256;

	struct ChunkEntry
	{
		char name[4];
		int dataSize;
	};

	enum EntryName { Unknown = -1, TileSet, Dimensions, Terrain_Gameplay };

	std::unordered_map<string, EntryName> nameMap = {
		{ "ERA ", TileSet },
		{ "DIM ", Dimensions },
		{ "MTXM", Terrain_Gameplay },
	};

	void ReadMap(std::shared_ptr<uint8_t[]> data, int dataSize, MapInfo& mapInfo)
	{
		StreamReader reader(data, dataSize);

		int tileAmount = 0;

		std::vector<string> ignoredEntries;

		while(!reader.IsEOF())
		{
			ChunkEntry nextEntry;
			reader.Read(nextEntry);

			int dataSize = nextEntry.dataSize;

			string nameString = string(nextEntry.name, 4);
			EntryName nameValue = nameMap.find(nameString) != nameMap.end() ? nameMap[nameString] : Unknown;	

			switch (nameValue) {
				case TileSet:
					reader.Read(mapInfo.tileset);
					break;

				case Dimensions:
					reader.Read(mapInfo.dimensions);
					break;

				case Terrain_Gameplay:

					assert(dataSize <= MAX_MAP_SIZE * MAX_MAP_SIZE * sizeof(uint16_t));

					tileAmount = dataSize / sizeof(uint16_t);
					mapInfo.terrain = std::make_shared<uint16_t[]>(tileAmount);

					reader.ReadBinary(mapInfo.terrain.get(), dataSize);
					break;

				default:
					ignoredEntries.push_back(nameString);

					reader.Skip(dataSize);
					break;
			}		
		}

		assert(tileAmount == mapInfo.dimensions.x * mapInfo.dimensions.y);

		// Remove when done with supporting of all possible entries
		std::cout << "Map reading: ";

		for(auto& name : ignoredEntries)
		{
			std::cout << boost::format("'%1%' ") % name;
		}

		std::cout << " entries are ignored" << std::endl;
	}
}