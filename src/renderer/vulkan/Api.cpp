#include "Api.hpp"

#include <glm/vec2.hpp>

using glm::vec2;

namespace renderer
{
	
	void Api::LoadGrp(grpID grpID)
	{
	/* Примерный алгоритм действий для чтения Grp спрайтов

	TextStringsTable imageStrings;
	data::ReadTextStringsTable(storage, "arr/images.tbl", imageStrings);
	uint8_t pixels[256 * 256];


		if (app.spriteAtlases.contains(doodad->grpID))
			continue;

		auto grpPath = imageStrings.entries[doodad->grpID];

		Grp grp;
		Grp::ReadGrpFile(storage, grpPath, grp);

		SpriteAtlas atlas;

		atlas.frames = grp.GetHeader().frameAmount;
		atlas.surfaces = new SDL_Surface*[atlas.frames];

		int frameIndex = 0;

		for(auto& frame : grp.GetFrames())
		{
			SDL_Surface* surface = SDL_CreateRGBSurface(0, frame.dimensions.x, frame.dimensions.y, 8, 0, 0, 0, 0);
			SDL_SetSurfacePalette(surface, app.tilesetAtlas.palette);

			SDL_LockSurface(surface);

			grp.GetFramePixels(frameIndex, pixels);

			int width = frame.dimensions.x;
			auto surfacePixels = reinterpret_cast<uint8_t*>(surface->pixels);

			for(int i = 0; i < surface->h; i++)
			{
				memcpy(surfacePixels + i * surface->pitch, pixels + i * width, width);
			}

			SDL_UnlockSurface(surface);
			SDL_SetColorKey(surface, SDL_TRUE, 0x00000000);

			atlas.rects.push_back(SDL_Rect { 
				.x = frame.posOffset.x - grp.GetHeader().dimensions.x / 2, .y = frame.posOffset.y - grp.GetHeader().dimensions.y / 2,
				.w = frame.dimensions.x, .h = frame.dimensions.y });
			atlas.surfaces[frameIndex++] = surface;
		}

		app.spriteAtlases[doodad->grpID] = atlas;*/
	}

	void Api::DrawGrpFrame(grpID grpID, uint32_t frame, vec2 position)
	{
		/* Примерный алгоритм отрисовки спрайта на экран
		
		auto spriteAtlas = spriteAtlases[grpID];
		auto surface     = spriteAtlas.surfaces[frame];
		auto destRect    = spriteAtlas.rects[frame];

		destRect.x += doodad->pos.x;
		destRect.y += doodad->pos.y;

		SDL_BlitSurface(surface, nullptr, app.screenSurface, &destRect);*/
	}

	void Api::CycleWaterPalette()
	{
		/* примерный алгоритм действий
		
		cyclePaletteColor<1, 6>(app.tilesetAtlas);
		cyclePaletteColor<7, 7>(app.tilesetAtlas);
		*/
	}
}