
#pragma once

#include "Atlas.hpp"

#include "../data/Tile.hpp"

namespace render
{
	template<int L>
	void createTilesetAtlas(data::TilesetData& tilesetData, GridAtlas& atlas)
	{
		auto palette = SDL_AllocPalette(256);
		auto colors = reinterpret_cast<const SDL_Color*>(tilesetData.paletteData.GetColors());

		SDL_SetPaletteColors(palette, colors, 0, 256);

		atlas = GridAtlas(L, data::TILE_SIZE, palette);

		for(int i = 0; i < tilesetData.tilesCount; i+=L*L)
		{
			const int width = L * data::TILE_SIZE;
			const int height = L * data::TILE_SIZE;
			auto surface = SDL_CreateRGBSurface(0, width, height, 8, 0, 0, 0, 0);
			SDL_SetSurfacePalette(surface, palette);

			SDL_LockSurface(surface);

			for(int j = 0; j < width; j+=8)
			for(int k = 0; k < height; k++)
			{
				data::tileID tileId = i + j / data::TILE_SIZE + k / data::TILE_SIZE * L;

				if (tileId >= tilesetData.tilesCount)
					continue;

				auto& tile = tilesetData.tiles[tileId];

				uint32_t chipId = tile.GetChipId(k / 8 % 4, j / 8 % 4);
				auto&    chip = tilesetData.chips[chipId];

				auto pixels = reinterpret_cast<uint8_t*>(surface->pixels) + j + k * width;

				if (tile.IsTileMirrored(k / 8 % 4, j / 8 % 4))
				{
					for(int n = 0; n < 8; n++)

						pixels[7 - n] = chip.palPixels[(k % 8) * 8 + n];
				}
				else
				{
					memcpy(pixels, &chip.palPixels[(k % 8) * 8], 8);
				}
			}

			SDL_UnlockSurface(surface);

			atlas.pages.push_back(surface);
		}
	}
}