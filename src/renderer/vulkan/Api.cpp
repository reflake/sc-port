#include "Api.hpp"
#include "VulkanGraphics.hpp"
#include "data/Assets.hpp"

#include <memory>

#include <SDL3/SDL_video.h>
#include <stdexcept>

namespace renderer::vulkan
{
	std::unique_ptr<A_Graphics> CreateGraphics(void* window, const data::Assets* assets)
	{
		SDL_Window* sdlWindow = reinterpret_cast<SDL_Window*>(window);

		if (sdlWindow == nullptr)
		{
			throw std::runtime_error("Instance of window is not supported");
		}

		return std::make_unique<Graphics>(sdlWindow, assets);
	}
}