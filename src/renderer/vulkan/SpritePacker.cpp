#include "SpritePacker.hpp"

namespace renderer::vulkan
{

	SpritePacker::SpritePacker(const std::vector<SpriteFrameData>& frameDatas) : _frameDatas(frameDatas)
	{
	}

	Atlas SpritePacker::CreateAtlas()
	{
		sprite packing algorithm goes there
	}
}