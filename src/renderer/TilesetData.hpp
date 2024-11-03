#pragma once

#include <memory>

namespace renderer
{
	class Tile
	{
	public:

		Tile(std::shared_ptr<uint8_t[]> pixelData);
	};

	class TilesetData
	{
	public:

		TilesetData(std::shared_ptr<Tile[]> tiles);
	};
}