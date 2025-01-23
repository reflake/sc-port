#pragma once

#include <vector>

#include "Atlas.hpp"
#include "data/Sprite.hpp"

namespace renderer::vulkan
{
	class SpritePacker
	{
	public:

		SpritePacker(const std::vector<data::SpriteData>& frameDatas);

		Atlas CreateAtlas();

	private:

		const std::vector<data::SpriteData>& _frameDatas;
	};
}