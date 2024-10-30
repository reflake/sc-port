#pragma once

#include <vector>

#include "Atlas.hpp"

#include "../SpriteFrameData.hpp"

namespace renderer::vulkan
{
	class SpritePacker
	{
	public:

		SpritePacker(const std::vector<SpriteFrameData>& frameDatas);

		Atlas CreateAtlas();

	private:

		const std::vector<SpriteFrameData>& _frameDatas;
	};
}