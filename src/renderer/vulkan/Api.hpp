#pragma once

#include "../A_Graphics.hpp"

#include <memory>

namespace renderer::vulkan
{
	extern std::unique_ptr<A_Graphics> CreateGraphics(void* window);
}
