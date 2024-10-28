#pragma once

#include "../A_SpriteSheet.hpp"
#include "../SpriteFrameData.hpp"

#include <glm/vec2.hpp>
#include <vector>

namespace renderer::vulkan
{
	class SpriteSheet : public A_SpriteSheet
	{
	public:

		SpriteSheet(std::vector<SpriteFrameData>& frameDatas, BufferAllocator&);

	private:

		std::vector<glm::vec<2, uint32_t>> _offsets;
		std::shared_ptr<Texture> _texture;
	};
}