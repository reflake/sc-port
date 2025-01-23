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
		SpriteSheetType, TilesetType
	};

	class A_VulkanDrawable
	{
	public:

		template<std::size_t L>
		inline std::size_t GetPolygon(frameIndex frame, std::array<Vertex, L>& array, std::size_t maxCount) 
		{
			return GetPolygon(frame, array.data(), maxCount);
		}

		virtual std::size_t GetPolygon(frameIndex, Vertex* output, std::size_t maxCount) const = 0;

		virtual DrawableType GetType() const = 0;

		virtual VkImageView GetImageView() const = 0;

		virtual Image* GetImage() const = 0;
	};

	class SpriteSheet : public A_VulkanDrawable
	{
	public:

		SpriteSheet(std::vector<data::SpriteData>&, Atlas&, Image*);

		std::size_t GetPolygon(frameIndex, Vertex* output, std::size_t maxCount) const override;

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

		std::size_t GetPolygon(frameIndex, Vertex* output, std::size_t maxCount) const override;

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
}