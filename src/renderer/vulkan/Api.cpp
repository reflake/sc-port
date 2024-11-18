#include "Api.hpp"
#include "VulkanGraphics.hpp"

#include <memory>

#include <SDL_video.h>
#include <stdexcept>

namespace renderer::vulkan
{
	std::unique_ptr<A_Graphics> CreateGraphics(void* window)
	{
		SDL_Window* sdlWindow = reinterpret_cast<SDL_Window*>(window);

		if (sdlWindow == nullptr)
		{
			throw std::runtime_error("Instance of window is not supported");
		}

		return std::make_unique<Graphics>(sdlWindow);
	}
}