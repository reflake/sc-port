#include "SpriteSheet.hpp"

namespace renderer::vulkan
{
	SpriteSheet::SpriteSheet(std::vector<SpriteFrameData>& frameDatas, BufferAllocator& bufferAllocator)
	{
		for(auto& frame : frameDatas)
		{
			_offsets.push_back(frame.GetOffset());
		}

		SpritePacker spritePacker;

		for(auto& frame : frameDatas)
		{
			spritePacker.Pack(frame.GetDimensions(), frame.GetPixelData());
		}

		auto atlas = spritePacker.CreateAtlas();

		_texture = bufferAllocator.CreateTexture(atlas.GetWidth(), atlas.GetHeight(), atlas.GetPixelData());
	}
}