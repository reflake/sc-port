#include "Video.hpp"

#include "webm/ParseCallback.hpp"
#include "webm/WebmReader.hpp"
#include <webm/webm_parser.h>

namespace video
{
	VideoManager::VideoManager(data::Assets* assets) :
		_assets(assets)
	{}

	bool VideoManager::OpenVideo(const char* path, VideoAsset* video)
	{
		video->assetHandle = _assets->Open(path);

		ParseCallback callback(*video);
		WebmReader reader(_assets, video->assetHandle);
		webm::WebmParser  parser;
		webm::Status      status = parser.Feed(&callback, &reader);

		return status.completed_ok();
	}

	void VideoManager::ReadFrameData(VideoAsset* video, FrameMeta& frame, uint8_t* output)
	{
		_assets->Seek(video->assetHandle, frame.offset, filesystem::FileSeekDir::Beg);
		_assets->ReadBytes(video->assetHandle, output, frame.size);
	}

	void VideoManager::FreeVideo(VideoAsset* video)
	{
		_assets->Close(video->assetHandle);
	}
}