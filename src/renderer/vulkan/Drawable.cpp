#include "Drawable.hpp"
#include "data/Tileset.hpp"

#include <cstring>
#include <iostream>
#include <utility>

namespace renderer::vulkan {

	using std::array;

	DrawableType SpriteSheet::GetType() const { return SpriteSheetType; }

	Tileset::Tileset(data::A_TilesetData& tilesetData, std::vector<uint32_t>& tileMap, const Image* image, int cellSize, int textureWidth, int textureHeight)
		: _tilesetData(tilesetData), _tileMap(tileMap), _image(image),
			 CellSize(cellSize), TextureWidth(textureWidth), TextureHeight(textureHeight)
	{
	}

	std::size_t Tileset::GetPolygon(frameIndex frameIndex, Vertex* output, std::size_t maxCount) const
	{
		bool flipped   = _tilesetData.GetFlipFlags(frameIndex) & data::FlipHorizontally;
		int  realIndex = _tilesetData.GetMappedIndex(frameIndex);

		realIndex = _tileMap[realIndex];

		uint32_t column = realIndex * CellSize % TextureWidth;
		uint32_t row    = (realIndex * CellSize / TextureWidth) * CellSize;
		float uvTop = static_cast<float>(row + CellSize) / TextureHeight;
		float uvBottom = static_cast<float>(row) / TextureHeight;
		float uvLeft = static_cast<float>(column) / TextureWidth;
		float uvRight = static_cast<float>(column + CellSize) / TextureWidth;

		if (flipped)
		{
			std::swap(uvLeft, uvRight);
		}

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