#pragma once

#include "Common.hpp"

#include <glm/vec2.hpp>
#include <memory>
#include <utility>
#include <vector>

#include "../filesystem/MpqArchive.hpp"

namespace data
{
	typedef glm::vec<2, uint16_t> dimensions;

	struct MapSprite;

	struct MapInfo
	{
		bool     onlyEditorInfo;
		char 		 mapType[4]; // equals either RAWS or RAWB
		uint16_t version;	
		Tileset  tileset;
		
		dimensions dimensions = { 0, 0 };

		std::shared_ptr<uint16_t[]> terrain;

		int tileCount;

		std::vector<MapSprite> sprites;

		MapInfo(bool onlyEditorInfo);

		std::pair<tileGroupID, tileVariation> GetTile(int x, int y);
	};

	struct MapSprite
	{
		uint16_t              spriteID;
		glm::vec<2, uint16_t> position;
		uint8_t               owner;
		uint8_t               unused;
		uint16_t              flags;
	};

	extern void ReadMap(filesystem::MpqArchive& mapArchive, MapInfo& mapInfo);

	extern const uint16_t MAX_MAP_SIZE;
}