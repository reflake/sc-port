#include "SoundStream.hpp"
#include <algorithm>

namespace audio
{
	SoundStream::SoundStream()
	{}

	SoundStream::SoundStream(data::Assets* assets, data::AssetHandle soundAsset, int fullSize, std::shared_ptr<uint8_t[]> initialBuffer, int bufferSize) :
		_assets(assets),
		_soundAsset(soundAsset),
		_fullSize(fullSize),
		_initialBuffer(initialBuffer),
		_bufferSize(bufferSize)
	{}

	int SoundStream::ReadChunk(int maxSize, uint8_t* output)
	{
		int bytesRead = 0;
		int dataLeft  = _fullSize - _fileOffset;
		maxSize = std::min(maxSize, dataLeft);

		// First, read from the buffer
		if (maxSize > 0 && _bufferSize > 0)
		{
			int size = std::min(maxSize, _bufferSize);

			memcpy(output, _initialBuffer.get() + _bufferOffset, size);

			_bufferOffset  += size;
			_bufferSize    -= size;

			bytesRead += size;
			maxSize   -= size;

			_assets->Seek(_soundAsset, size, filesystem::FileSeekDir::Cur);
		}

		// Then from external sources
		if (maxSize > 0)
		{
			bytesRead += _assets->ReadBytes(_soundAsset, output + bytesRead, maxSize);
		}

		_fileOffset += bytesRead;

		return bytesRead;
	}

	bool SoundStream::IsEndOfStream() { return _fileOffset >= _fullSize; }

	void SoundStream::Release()
	{
		_assets->Close(_soundAsset);
	}
}