#pragma once

#include "Atlas.hpp"
#include "Texture.hpp"

#include <memory>

namespace renderer::vulkan
{
	class BufferAllocator
	{
	public:

		std::unique_ptr<Texture> CreateTexture(Atlas& atlas);
	};
}