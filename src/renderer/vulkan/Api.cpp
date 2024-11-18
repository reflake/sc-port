#include "Api.hpp"
#include "VulkanGraphics.hpp"

#include <memory>

#include <SDL_video.h>

namespace renderer::vulkan
{
	std::unique_ptr<A_Graphics> CreateGraphics(void* window)
	{
		SDL_Window* sdlWindow = reinterpret_cast<SDL_Window*>(window);

		return std::make_unique<Graphics>(sdlWindow);
	}
}