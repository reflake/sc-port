#include "SpritePacker.hpp"
#include "SpriteSheet.hpp"

namespace renderer::vulkan
{
	SpriteSheet::SpriteSheet(std::vector<SpriteFrameData>& frameDatas, BufferAllocator& bufferAllocator)
	{
		for(auto& frame : frameDatas)
		{
			_offsets.push_back(frame.GetOffset());
		}

		SpritePacker spritePacker(frameDatas);

		auto atlas = spritePacker.CreateAtlas();

		_atlas   = atlas;
		_texture = bufferAllocator.CreateTexture(atlas);
	}

	const Frame SpriteSheet::GetFrame(int frameIndex) const
	{
		return Frame { 
			_atlas.GetMinXY(frameIndex), _atlas.GetMaxXY(frameIndex),
			_offsets[frameIndex], _texture
		};
	}
}