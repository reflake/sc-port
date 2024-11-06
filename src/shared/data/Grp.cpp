#include "Grp.hpp"

#include <cstring>
#include <string>

#include "Common.hpp"

namespace data
{

	const SpriteData Grp::GetSpriteData(int frameIndex) const
	{
		const int numOfChannels = 1;

		auto frame = _frames[frameIndex];

		return { numOfChannels, frame.posOffset, frame.dimensions };
	}

	const int Grp::ReadPixelData(int frameIndex, uint8_t* out) const
	{
		const int TRANSPARENT_FLAG = 0x80;
		const int REPEAT_FLAG = 0x40;

		const GrpFrame& frame = _frames[frameIndex];
		uint16_t* rleLinesOffsets = reinterpret_cast<uint16_t*>(_data.get() + frame.linesOffset);

		memset(out, 0, frame.dimensions.x * frame.dimensions.y);

		for(int y = 0; y < frame.dimensions.y; y++)
		{
			auto rleLines = reinterpret_cast<uint8_t*>(rleLinesOffsets) + rleLinesOffsets[y];
			auto pixelsRow = &out[y * frame.dimensions.x];

			for(int x = 0; x < frame.dimensions.x; )
			{
				auto flag = *rleLines++;

				if (flag & TRANSPARENT_FLAG)
				{
					x += flag & ~TRANSPARENT_FLAG;
				}
				else if (flag & REPEAT_FLAG)
				{
					auto length     = flag & ~REPEAT_FLAG;
					auto colorIndex = *rleLines++;

					memset(pixelsRow + x, colorIndex, length);

					x += length;
				}
				else
				{
					for(int l = 0; l < flag; l++)

						pixelsRow[x++] = *rleLines++;
				}
			}
		}
	}

	glm::vec<2, int> Grp::GetDimensionsLimit() const
	{
		return { GRP_DIMENSIONS_LIMIT, GRP_DIMENSIONS_LIMIT };
	}

	const GrpHeader& Grp::GetHeader() const
	{
		return _header;
	}
	
	const GrpFrame& Grp::GetFrame(int frame) const
	{
		return _frames[frame];
	}
	
	const std::vector<GrpFrame>& Grp::GetFrames() const
	{
		return _frames;
	}
	
	Grp Grp::ReadGrpFile(filesystem::Storage& storage, const char* path)
	{
		std::string fullpath = "unit\\" + std::string(path);

		filesystem::StorageFile file;
		storage.Open(fullpath.c_str(), file);

		auto data = std::make_shared<uint8_t[]>(file.GetFileSize());
		file.ReadBinary(data.get(), file.GetFileSize());

		return ReadGrp(data, file.GetFileSize());
	}

	Grp Grp::ReadGrp(std::shared_ptr<uint8_t[]> data, int size)
	{
		auto reader = StreamReader(data, size);
		Grp out;	

		reader.Read(out._header);

		out._frames.resize(out._header.frameAmount);

		reader.Read(out._frames.data(), out._header.frameAmount);

		out._data = data;

		return out;
	}
}