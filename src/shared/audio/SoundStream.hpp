#pragma once

#include "data/Assets.hpp"
#include <cstdint>

namespace audio
{
	class SoundStream
	{
	public:

		SoundStream();
		SoundStream(data::Assets*, data::AssetHandle, int fullSize, std::shared_ptr<uint8_t[]> initialBuffer = nullptr, int bufferSize = 0);

		int ReadChunk(int maxSize, uint8_t* output);

		bool IsEndOfStream();

		void Release();

	private:

		data::Assets*     _assets = nullptr;
		data::AssetHandle _soundAsset = nullptr;

		int _fullSize = 0;
		int _fileOffset = 0;
		int _bufferSize = 0;
		int _bufferOffset = 0;

		std::shared_ptr<uint8_t[]> _initialBuffer = nullptr;
	};
}