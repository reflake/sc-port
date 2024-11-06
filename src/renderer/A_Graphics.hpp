#pragma once

#include "A_Tileset.hpp"
#include "A_SpriteSheet.hpp"
#include "SpriteFrameData.hpp"

#include "data/Palette.hpp"
#include <data/Tileset.hpp>

#include <cstdint>
#include <glm/vec2.hpp>

namespace renderer
{
	typedef uint32_t grpID;
	typedef uint16_t tileID;

	class A_Graphics
	{
	public:

		virtual const A_SpriteSheet* CreateSpriteSheet(std::shared_ptr<SpriteFrameData> frames) = 0;
		virtual void DrawSprite(const A_SpriteSheet*, uint32_t frame, glm::vec2 position) = 0;
		virtual void FreeSpriteSheet(const A_SpriteSheet*) = 0;

		virtual const A_Tileset* LoadTileset(data::A_TilesetData&) = 0;
		virtual void DrawTile(const A_Tileset*, tileID, glm::vec2 position) = 0;
		virtual void FreeTileset(const A_Tileset*) = 0;

		virtual void SetTilesetPalette(data::Palette) = 0;

		virtual void ClearDepth();
		virtual void PresentToScreen();

		virtual const char* GetName() const;
	};
}