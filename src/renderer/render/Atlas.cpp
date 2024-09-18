#include "Atlas.hpp"

#include <utility>

using std::pair;

namespace render
{
	pair<SDL_Surface*, SDL_Rect> GridAtlas::GetTile(uint16_t id) const
	{
		int pageIndex = id / dimension / dimension;

		id = id - pageIndex * dimension * dimension;

		SDL_Rect rect = { 
			.x = (id % dimension) * tileSize, 
			.y = id / dimension * tileSize,
			.w = tileSize, 
			.h = tileSize };

		return { pages[pageIndex], rect };
	}

	void GridAtlas::Free()
	{
		for(auto page : pages)
		{
			SDL_FreeSurface(page);
		}

		if (palette != nullptr)
			SDL_FreePalette(palette);

		dimension = 0;
		tileSize = 0;
		palette = nullptr;
		pages.clear();
	}
}