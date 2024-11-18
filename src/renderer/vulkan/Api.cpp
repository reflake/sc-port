#include "Api.hpp"
#include "VulkanGraphics.hpp"
#include "filesystem/Storage.hpp"

#include <memory>

#include <SDL_video.h>

namespace renderer::vulkan
{
	std::unique_ptr<A_Graphics> CreateGraphics(void* window, void* storage)
	{
		SDL_Window* sdlWindow = reinterpret_cast<SDL_Window*>(window);
		filesystem::Storage& storageReference = *reinterpret_cast<filesystem::Storage*>(storage);

		return std::make_unique<Graphics>(sdlWindow, storageReference);
	}
}