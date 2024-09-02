#pragma once

#include "Common.hpp"

#include <glm/vec2.hpp>
#include <memory>


namespace data
{
	typedef glm::vec<2, uint16_t> dimensions;

	struct MapInfo
	{
		Tileset tileset;
		dimensions dimensions;

		std::shared_ptr<uint16_t[]> terrain;
	};

	extern void ReadMap(std::shared_ptr<uint8_t[]> data, int dataSize, MapInfo& mapInfo);

	extern const uint16_t MAX_MAP_SIZE;
}