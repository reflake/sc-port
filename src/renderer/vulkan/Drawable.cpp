#include "Drawable.hpp"
#include "data/Tileset.hpp"

#include <cstring>
#include <iostream>
#include <utility>

#include <data/Vertex.hpp>

namespace renderer::vulkan {

	DrawableType SpriteSheet::GetType() const { return SpriteSheetType; }

	Tileset::Tileset(data::A_TilesetData& tilesetData, std::vector<uint32_t>& tileMap, const Image* image, int cellSize, int textureWidth, int textureHeight)
		: _tilesetData(tilesetData), _tileMap(tileMap), _image(image),
			 CellSize(cellSize), TextureWidth(textureWidth), TextureHeight(textureHeight)
	{
	}

	std::size_t Tileset::GetPolygon(frameIndex frameIndex, Vertex* output, std::size_t maxCount) const
	{
		auto flips     = _tilesetData.GetFlipFlags(frameIndex);
		int  realIndex = _tilesetData.GetMappedIndex(frameIndex);

		realIndex = _tileMap[realIndex];

		uint32_t texLeft  = realIndex * CellSize % TextureWidth;
		uint32_t texRight = (realIndex * CellSize / TextureWidth) * CellSize;

		auto [bottomLeft, topLeft, topRight, bottomRight] = data::FrameVertices<Vertex>(0, 0, CellSize, CellSize, texLeft, texRight, TextureWidth, TextureHeight, flips);

		output[0] = bottomLeft;
		output[1] = topLeft;
		output[2] = topRight;
		output[3] = bottomLeft;
		output[4] = topRight;
		output[5] = bottomRight;

		return 6;
	}

	VkImageView Tileset::GetImageView() const
	{
		return _image->GetViewHandle();
	}

	DrawableType Tileset::GetType() const { return TilesetType; }
}