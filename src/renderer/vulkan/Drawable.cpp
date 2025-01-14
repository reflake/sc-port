#include "Drawable.hpp"

#include <cstring>

namespace renderer::vulkan {

	using std::array;

	DrawableType SpriteSheet::GetType() const { return SpriteSheetType; }

	Tileset::Tileset(int cellSize)
		: CellSize(cellSize) {}

	std::size_t Tileset::GetPolygon(frameIndex frameIndex, Vertex* output, std::size_t maxCount) const
	{
		// TODO: frame index is unused

		array<Vertex, 6> quad = { 
			Vertex( { 0.0f, 0.0f },   { 0, 0 } ),
			Vertex( { 0.0f, CellSize },  { 0, 1 } ),
			Vertex( { CellSize, CellSize }, { 1, 1 } ),
			Vertex( { 0.0f, 0.0f },   { 0, 0 } ),
			Vertex( { CellSize, 0.0f },  { 1, 0 } ),
			Vertex( { CellSize, CellSize }, { 1, 1 } ),
		};

		memcpy(output, quad.data(), std::min(maxCount, quad.size()) * sizeof(Vertex));

		return quad.size();
	}

	DrawableType Tileset::GetType() const { return TilesetType; }
}