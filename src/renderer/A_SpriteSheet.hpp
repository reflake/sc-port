#pragma once

#include <cstdint>
#include <memory>

#include "SpriteFrame.hpp"

namespace renderer
{
	class A_SpriteSheet
	{
	public:

		virtual std::shared_ptr<SpriteFrame> GetFrameData(uint32_t index) = 0;
	};
};