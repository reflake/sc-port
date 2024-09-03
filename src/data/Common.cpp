#include "Common.hpp"

namespace data
{
	bool HasTileSetWater(Tileset tileset)
	{
		switch (tileset) {
			case Tileset::Jungle:
				return true;
			case Tileset::SpacePlatform:
				return false;
			default:
				return false;
				//throw std::runtime_error("Not implemented");
		}
	}
}