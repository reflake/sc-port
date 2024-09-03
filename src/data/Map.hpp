#pragma once

#include "Common.hpp"

#include <glm/vec2.hpp>
#include <memory>
#include <utility>

namespace data
{
	typedef glm::vec<2, uint16_t> dimensions;

	struct MapInfo
	{
		char 		 mapType[4]; // equals either RAWS or RAWB
		uint16_t version;	
		Tileset  tileset;
		
		dimensions dimensions;

		std::shared_ptr<uint16_t[]> terrain;

		std::pair<tileGroupID, tileVariation> GetTile(int x, int y);
	};

	extern void ReadMap(std::shared_ptr<uint8_t[]> data, int dataSize, MapInfo& mapInfo, bool isEditor);

	extern const uint16_t MAX_MAP_SIZE;
}