#pragma once

#include "Common.hpp"

#include <glm/vec2.hpp>
#include <memory>
#include <utility>

#include "../filesystem/MpqArchive.hpp"

namespace data
{
	typedef glm::vec<2, uint16_t> dimensions;

	struct MapInfo
	{
		bool     onlyEditorInfo;
		char 		 mapType[4]; // equals either RAWS or RAWB
		uint16_t version;	
		Tileset  tileset;
		
		dimensions dimensions = { 0, 0 };

		std::shared_ptr<uint16_t[]> terrain;

		int tileCount;

		MapInfo(bool onlyEditorInfo);

		std::pair<tileGroupID, tileVariation> GetTile(int x, int y);
	};

	extern void ReadMap(filesystem::MpqArchive& mapArchive, MapInfo& mapInfo);

	extern const uint16_t MAX_MAP_SIZE;
}