#include "Drawable.hpp"
#include "data/Common.hpp"
#include "data/Sprite.hpp"
#include "data/Tileset.hpp"

#include <cstring>
#include <glm/ext/vector_float2.hpp>
#include <iostream>
#include <utility>

#include <data/Vertex.hpp>

namespace renderer::vulkan {

	// =========================================
	//   Sprite sheet
	//  TODO: description
	// =========================================
	SpriteSheet::SpriteSheet(std::vector<data::SpriteData>& spriteDataList, Atlas& atlas, Image* image) : 
		_spriteDataList(std::move(spriteDataList)),
		_atlas(std::move(atlas)),
		_spriteAtlasImage(image)
	{}

	std::size_t SpriteSheet::GetPolygon(frameIndex frameIndex, Vertex* output, std::size_t maxCount, uint32_t width, uint32_t height) const
	{
		auto spriteData = _spriteDataList[frameIndex];
		auto frameData  = _atlas.GetFrame(frameIndex);
		auto atlasDims  = _atlas.GetDimensions();

		width = width ? width : frameData.w;
		height = height ? height : frameData.h;

		auto [bottomLeft, topLeft, topRight, bottomRight] = data::FrameVertices<Vertex>(
			spriteData.offset.x, spriteData.offset.y, 
			width, height,
			frameData,
			atlasDims.x, atlasDims.y, 
			data::FlipNone);

		output[0] = bottomLeft;
		output[1] = topLeft;
		output[2] = topRight;
		output[3] = bottomLeft;
		output[4] = topRight;
		output[5] = bottomRight;

		return 6;

	}

	VkImageView SpriteSheet::GetImageView() const { return _spriteAtlasImage->GetViewHandle(); }

	Image* SpriteSheet::GetImage() const { return _spriteAtlasImage; }

	DrawableType SpriteSheet::GetType() const { return SpriteSheetType; }

	// =========================================
	//   Tileset
	//  TODO: description
	// =========================================
	Tileset::Tileset(data::A_TilesetData& tilesetData, std::vector<uint32_t>& tileMap, Image* image, int cellSize, int textureWidth, int textureHeight)
		: _tilesetData(tilesetData), _tileMap(tileMap), _image(image),
			 CellSize(cellSize), TextureWidth(textureWidth), TextureHeight(textureHeight)
	{
	}

	std::size_t Tileset::GetPolygon(frameIndex frameIndex, Vertex* output, std::size_t maxCount, uint32_t width, uint32_t height) const
	{
		auto flips     = _tilesetData.GetFlipFlags(frameIndex);
		int  realIndex = _tilesetData.GetMappedIndex(frameIndex);
		
		realIndex = _tileMap[realIndex];

		uint32_t texLeft  = realIndex * CellSize % TextureWidth;
		uint32_t texTop   = (realIndex * CellSize / TextureWidth) * CellSize;

		data::SpriteRect sprRect = { texLeft, texTop, static_cast<uint32_t>(CellSize), static_cast<uint32_t>(CellSize) };

		width = width ? width : CellSize;
		height = height ? height : CellSize;

		auto [bottomLeft, topLeft, topRight, bottomRight] = data::FrameVertices<Vertex>(0, 0, width, height, sprRect, TextureWidth, TextureHeight, flips);

		output[0] = bottomLeft;
		output[1] = topLeft;
		output[2] = topRight;
		output[3] = bottomLeft;
		output[4] = topRight;
		output[5] = bottomRight;

		return 6;
	}

	VkImageView Tileset::GetImageView() const { return _image->GetViewHandle(); }

	Image* Tileset::GetImage() const { return _image; }

	DrawableType Tileset::GetType() const { return TilesetType; }

	Picture::Picture(uint32_t width, uint32_t height, uint32_t texWidth, uint32_t texHeight, Image* image) :
		_width(width), _height(height), 
		_texWidth(texWidth), _texHeight(texHeight),
		_image(image)
	{}

	std::size_t Picture::GetPolygon(frameIndex, Vertex* output, std::size_t maxCount, uint32_t width, uint32_t height) const
	{
		data::SpriteRect sprRect = { 0, 0, _width, _height };

		auto [bottomLeft, topLeft, topRight, bottomRight] = data::FrameVertices<Vertex>(
			
			0, 0, 
			width ? width : _width, 
			height ? height : _height, 
			sprRect, 
			_texWidth, _texHeight, 
			data::FlipNone);

		output[0] = bottomLeft;
		output[1] = topLeft;
		output[2] = topRight;
		output[3] = bottomLeft;
		output[4] = topRight;
		output[5] = bottomRight;
		
		return 6;
	}

	DrawableType Picture::GetType() const { return PictureType; }

	VkImageView Picture::GetImageView() const { return _image->GetViewHandle(); }

	Image* Picture::GetImage() const { return _image; };
}