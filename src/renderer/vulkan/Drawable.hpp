#pragma once

#include <array>
#include <cstddef>
#include <stdexcept>

#include "../A_Graphics.hpp"

#include "Vertex.hpp"

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
	};

	class SpriteSheet : public A_VulkanDrawable
	{
	public:

		std::size_t GetPolygon(frameIndex, Vertex* output, std::size_t maxCount) const override { throw std::runtime_error("not implemented"); };

		DrawableType GetType() const override;
	};

	class Tileset : public A_VulkanDrawable
	{
	public:

		Tileset(int cellSize, int textureLength);

		std::size_t GetPolygon(frameIndex, Vertex* output, std::size_t maxCount) const override;

		DrawableType GetType() const override;

		const int CellSize;
		const int TextureLength;
	};
}