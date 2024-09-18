#pragma once

#include "Atlas.hpp"

namespace render
{
	// shifts some palette's colors so that's creates
	//  an illusion of animation
	template<int StartIndex, int Length>
	void cyclePaletteColor(GridAtlas& atlas)
	{
		SDL_Color newPaletteColor[Length];
		memcpy(newPaletteColor + 1, atlas.palette->colors + StartIndex, (Length - 1) * sizeof(SDL_Color));

		newPaletteColor[0] = atlas.palette->colors[StartIndex + Length - 1];

		SDL_SetPaletteColors(atlas.palette, newPaletteColor, StartIndex, Length);
	}
}