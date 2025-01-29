#pragma once

#include <array>
#include <cstddef>
#include <stdexcept>
#include <vulkan/vulkan_core.h>

#include "../A_Graphics.hpp"

#include "Atlas.hpp"
#include "Vertex.hpp"
#include "data/Sprite.hpp"
#include "data/Tileset.hpp"
#include "memory/Image.hpp"

namespace renderer::vulkan
{
	enum DrawableType
	{
		SpriteSheetType, TilesetType, PictureType
	};

	class A_VulkanDrawable
	{
	public:

		template<std::size_t L>
		inline std::size_t GetPolygon(frameIndex frame, std::array<Vertex, L>& array, std::size_t maxCount, uint32_t width = 0, uint32_t height = 0) 
		{
			return GetPolygon(frame, array.data(), maxCount, width, height);
		}

		virtual std::size_t GetPolygon(frameIndex, Vertex* output, std::size_t maxCount, uint32_t width = 0, uint32_t height = 0) const = 0;

		virtual DrawableType GetType() const = 0;

		virtual VkImageView GetImageView() const = 0;

		virtual Image* GetImage() const = 0;
	};

	class SpriteSheet : public A_VulkanDrawable
	{
	public:

		SpriteSheet(std::vector<data::SpriteData>&, Atlas&, Image*);

		std::size_t GetPolygon(frameIndex, Vertex* output, std::size_t maxCount, uint32_t width = 0, uint32_t height = 0) const override;

		DrawableType GetType() const override;

		VkImageView GetImageView() const override;

		Image* GetImage() const override;

	private:

		Atlas  _atlas;
		Image* _spriteAtlasImage;
		std::vector<data::SpriteData> _spriteDataList;
	};

	class Tileset : public A_VulkanDrawable
	{
	public:

		Tileset(data::A_TilesetData&, std::vector<uint32_t>& tileMap, Image*, int cellSize, int textureWidth, int textureHeight);

		std::size_t GetPolygon(frameIndex, Vertex* output, std::size_t maxCount, uint32_t width = 0, uint32_t height = 0) const override;

		DrawableType GetType() const override;

		VkImageView GetImageView() const override;

		Image* GetImage() const override;

		const int CellSize;
		const int TextureWidth, TextureHeight;

	private:

		data::A_TilesetData&   _tilesetData;
		std::vector<uint32_t>& _tileMap;

		Image* _image;
	};

	class Picture : public A_VulkanDrawable
	{
	public:

		Picture(uint32_t width, uint32_t height, uint32_t textureWidth, uint32_t textureHeight, Image*);

		std::size_t GetPolygon(frameIndex, Vertex* output, std::size_t maxCount, uint32_t width = 0, uint32_t height = 0) const override;

		DrawableType GetType() const override;

		VkImageView GetImageView() const override;

		Image* GetImage() const override;

	private:

		uint32_t _texWidth, _texHeight;
		uint32_t _width, _height;
		Image* _image;
	};
}