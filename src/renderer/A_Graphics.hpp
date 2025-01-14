#pragma once

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
	typedef void*    DrawableHandle;

	class A_Graphics
	{
	public:

		virtual DrawableHandle LoadSpriteSheet(data::A_SpriteSheetData&) = 0;
		virtual DrawableHandle LoadTileset(data::A_TilesetData&) = 0;

		virtual void Draw(DrawableHandle, frameIndex, data::position) = 0;
		virtual void FreeDrawable(DrawableHandle) = 0;

		virtual void SetTilesetPalette(data::Palette&) = 0;

		virtual void SetView(data::position pos) = 0;
		virtual void BeginRendering() = 0;
		virtual void PresentToScreen() = 0;

		virtual const char* GetName() const = 0;

		virtual void WaitIdle() = 0;

		virtual void Release() = 0;
	};
}