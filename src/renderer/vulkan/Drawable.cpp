#include "Drawable.hpp"

#include <cstring>

namespace renderer::vulkan {

	using std::array;

	DrawableType SpriteSheet::GetType() const { return SpriteSheetType; }

	Tileset::Tileset(const Image* image, int cellSize, int textureLength)
		: _image(image), CellSize(cellSize), TextureLength(textureLength)
	{
	}

	std::size_t Tileset::GetPolygon(frameIndex frameIndex, Vertex* output, std::size_t maxCount) const
	{
		const uint32_t column = frameIndex * CellSize % TextureLength;
		const uint32_t row    = (frameIndex * CellSize / TextureLength) * CellSize;
		const float uvTop = static_cast<float>(row + CellSize) / TextureLength;
		const float uvBottom = static_cast<float>(row) / TextureLength;
		const float uvLeft = static_cast<float>(column) / TextureLength;
		const float uvRight = static_cast<float>(column + CellSize) / TextureLength;

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