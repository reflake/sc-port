#pragma once

#include <cstdint>
#include <memory>

namespace renderer
{
	class TilesetData
	{
	public:

		TilesetData(const uint8_t* pixelData, int tileCount, int size);

	private:

		int _tileCount;
	};
}