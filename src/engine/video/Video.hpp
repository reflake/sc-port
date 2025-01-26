#pragma once

#include "data/Assets.hpp"
#include <cstdint>
#include <vector>

namespace video
{
	struct FrameMeta
	{
		std::uint64_t offset, size;
	};

	struct VideoAsset
	{
		int width, height;
		data::AssetHandle assetHandle = nullptr;

		std::vector<FrameMeta> frames;
	};

	class VideoManager
	{
	public:

		VideoManager(data::Assets*);

		bool OpenVideo(const char* path, VideoAsset*);

		// Reads encoded frame data
		void ReadFrameData(VideoAsset*, FrameMeta& frame, uint8_t* output);

		// Decodes frame data into pixels
		void DecodeFrameData(uint8_t* data, int frame);

		void FreeVideo(VideoAsset*);
	
	private:

		data::Assets* _assets;
	};
}

