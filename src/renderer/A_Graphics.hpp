#pragma once

#include "A_Tileset.hpp"
#include "A_SpriteSheet.hpp"

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

		virtual const A_SpriteSheet* LoadSpriteSheet(data::A_SpriteSheetData&) = 0;
		virtual void DrawSprite(const A_SpriteSheet*, frameIndex, glm::vec2 position) = 0;
		virtual void FreeSpriteSheet(const A_SpriteSheet*) = 0;

		virtual const A_Tileset* LoadTileset(data::A_TilesetData&) = 0;
		virtual void DrawTile(const A_Tileset*, tileID, glm::vec2 position) = 0;
		virtual void FreeTileset(const A_Tileset*) = 0;

		virtual void SetTilesetPalette(data::Palette) = 0;

		virtual void ClearDepth() = 0;
		virtual void PresentToScreen() = 0;

		virtual const char* GetName() const;
	};
}