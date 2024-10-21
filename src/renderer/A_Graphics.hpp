#pragma once

#include "SpriteFrameData.hpp"
#include "A_SpriteSheet.hpp"
#include <cstdint>
#include <glm/vec2.hpp>

#include <data/Tileset.hpp>

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

		virtual void LoadTileset(data::Tileset) = 0;
		virtual void DrawTile(data::Tileset, tileID, glm::vec2 position) = 0;
		virtual void FreeTileset(data::Tileset) = 0;

		virtual void SetTilesetPalette(data::Tileset) = 0;

		virtual void CycleWaterPalette();

		virtual void ClearDepth();
		virtual void PresentToScreen();

		virtual const char* GetName() const;
	};
}