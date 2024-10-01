#include <cstdint>

namespace renderer
{
	enum class Tileset: uint16_t { 
		Badlands, SpacePlatform, Installation, 
		Ashworld, Jungle, Desert, Arctic,
		Twilight
	};

	extern bool HasTileSetWater(Tileset tileset);
}