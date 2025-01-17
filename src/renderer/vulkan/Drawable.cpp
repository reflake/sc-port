#include "Drawable.hpp"

#include <cstring>
#include <iostream>

namespace renderer::vulkan {

	using std::array;

	DrawableType SpriteSheet::GetType() const { return SpriteSheetType; }

	Tileset::Tileset(const Image* image, int cellSize, int textureWidth, int textureHeight)
		: _image(image), CellSize(cellSize), TextureWidth(textureWidth), TextureHeight(textureHeight)
	{
	}

	std::size_t Tileset::GetPolygon(frameIndex frameIndex, Vertex* output, std::size_t maxCount) const
	{
		const uint32_t column = frameIndex * CellSize % TextureWidth;
		const uint32_t row    = (frameIndex * CellSize / TextureWidth) * CellSize;
		const float uvTop = static_cast<float>(row + CellSize) / TextureHeight;
		const float uvBottom = static_cast<float>(row) / TextureHeight;
		const float uvLeft = static_cast<float>(column) / TextureWidth;
		const float uvRight = static_cast<float>(column + CellSize) / TextureWidth;

		array<Vertex, 6> quad = { 
			Vertex( { 0.0f, 0.0f },         { uvLeft,  uvBottom } ),
			Vertex( { 0.0f, CellSize },     { uvLeft,  uvTop    } ),
			Vertex( { CellSize, CellSize }, { uvRight, uvTop    } ),
			Vertex( { 0.0f, 0.0f },         { uvLeft,  uvBottom } ),
			Vertex( { CellSize, 0.0f },     { uvRight, uvBottom } ),
			Vertex( { CellSize, CellSize }, { uvRight, uvTop    } ),
		};

		memcpy(output, quad.data(), std::min(maxCount, quad.size()) * sizeof(Vertex));

		return quad.size();
	}

	VkImageView Tileset::GetImageView() const
	{
		return _image->GetViewHandle();
	}

	DrawableType Tileset::GetType() const { return TilesetType; }
}