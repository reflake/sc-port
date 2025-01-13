#pragma once

#include "A_Drawable.hpp"

#include "data/Common.hpp"
#include "data/Palette.hpp"
#include <data/Tileset.hpp>
#include <data/Sprite.hpp>

#include <cstdint>
#include <glm/vec2.hpp>

namespace renderer
{
	typedef uint32_t frameIndex;
	typedef uint32_t tileID;

	class A_Graphics
	{
	public:

		virtual ~A_Graphics() = 0;

		virtual const A_Drawable* LoadSpriteSheet(data::A_SpriteSheetData&) = 0;
		virtual const A_Drawable* LoadTileset(data::A_TilesetData&) = 0;

		virtual void Draw(const A_Drawable*, frameIndex, data::position) = 0;
		virtual void FreeDrawable(const A_Drawable*) = 0;

		virtual void SetTilesetPalette(data::Palette&) = 0;

		void SetView(data::position pos);
		virtual void BeginRendering() = 0;
		virtual void PresentToScreen() = 0;

		virtual const char* GetName() const;

		void WaitIdle();
	};
}