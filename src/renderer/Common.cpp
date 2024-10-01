#include "Common.hpp"

#include <stdexcept>

namespace renderer
{
	bool HasTileSetWater(Tileset tileset)
	{
		switch (tileset) {
			case Tileset::Jungle:
			case Tileset::Twilight:
			case Tileset::Arctic:
			case Tileset::Ashworld: // not water actually but magma
			case Tileset::Badlands:
			case Tileset::Desert:
				return true;
			case Tileset::SpacePlatform:
			case Tileset::Installation:
				return false;
		}

		throw new std::runtime_error("Not implemented");
	}
}