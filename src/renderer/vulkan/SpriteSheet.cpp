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

		_atlas   = spritePacker.CreateAtlas();
		_texture = bufferAllocator.CreateTexture(_atlas);
	}

	const Frame SpriteSheet::GetFrame(int frameIndex) const
	{
		return Frame { 
			_atlas.GetMinXY(frameIndex), _atlas.GetMaxXY(frameIndex),
			_offsets[frameIndex], _texture
		};
	}
}