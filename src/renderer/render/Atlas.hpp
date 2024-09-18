#pragma once

#include <SDL_pixels.h>
#include <SDL_surface.h>

#include <utility>
#include <vector>

namespace render
{
	struct GridAtlas
	{
		int dimension = 0;
		int tileSize = 0;
		SDL_Palette* palette = nullptr;
		std::vector<SDL_Surface*> pages;

		std::pair<SDL_Surface*, SDL_Rect> GetTile(uint16_t id) const;

		void Free();
	};
}