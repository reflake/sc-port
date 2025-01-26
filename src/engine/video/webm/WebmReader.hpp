
#pragma once

#include <webm/reader.h>
#include <webm/status.h>

#include "data/Assets.hpp"

namespace video
{
	// ===============================
	//   WebmReader 
	// 
	// The class that handles reading webp metadata of game video assets
	//
	// ===============================
	class WebmReader : public webm::Reader
	{
	public:

		WebmReader(data::Assets* assets, data::AssetHandle videoFileHandle) :
			_assets(assets),
			_videoFileHandle(videoFileHandle)
		{
		}

		webm::Status Read(std::size_t size, std::uint8_t* buffer, std::uint64_t* bytesRead) override;
		webm::Status Skip(std::uint64_t bytesToSkip, std::uint64_t* actuallySkipped) override;
		std::uint64_t Position() const override;

	private:

		data::Assets* _assets;
		data::AssetHandle _videoFileHandle;
	};
}