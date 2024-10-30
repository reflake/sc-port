#pragma once

#include "../A_SpriteSheet.hpp"
#include "../SpriteFrameData.hpp"

#include "Atlas.hpp"
#include "BufferAllocator.hpp"
#include "Texture.hpp"

#include <glm/vec2.hpp>
#include <vector>

namespace renderer::vulkan
{
	struct Frame
	{
		glm::vec<2, uint32_t> minXY;
		glm::vec<2, uint32_t> maxXY;
		glm::vec<2, uint32_t> offset;
		std::shared_ptr<Texture> texture;
	};

	class SpriteSheet : public A_SpriteSheet
	{
	public:

		SpriteSheet(std::vector<SpriteFrameData>& frameDatas, BufferAllocator&);

		const Frame GetFrame(int index) const;

	private:

		Atlas _atlas;

		std::vector<glm::vec<2, uint32_t>> _offsets;
		std::shared_ptr<Texture> _texture;
	};
}