#include <array>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>

#include <string>
#include <vector>
#include <vpx/vpx_codec.h>
#include <vpx/vpx_image.h>
#include <webm/callback.h>
#include <webm/element.h>
#include <webm/id.h>
#include <webm/reader.h>
#include <webm/status.h>
#include <webm/file_reader.h>
#include <webm/webm_parser.h>
#include <webm/dom_types.h>

#include "data/Assets.hpp"
#include "diagnostic/Image.hpp"
#include "filesystem/StorageFile.hpp"
#include "vpx/vpx_decoder.h"
#include <vpx/vp8dx.h>

#include <filesystem/Storage.hpp>

struct Video
{
	int width, height;
	int frameMaxSize = 0;

	std::vector<webm::FrameMetadata> frameMetas;
};

class VideoCallback : public webm::Callback
{
public:

	VideoCallback(Video& video) :
		_video(video)
	{}

	template<typename T>
	const T& GetEntry(const webm::Element<T>& track_entry)
	{
		if (!track_entry.is_present())
		{
			throw std::runtime_error("Invalid entry");
		}

		return track_entry.value();
	}

	template<typename T>
	const T GetValue(const webm::Element<T> value_element)
	{
		if (!value_element.is_present())
		{
			throw std::runtime_error("Invalid value element");
		}

		return value_element.value();
	}

  webm::Status OnTrackEntry(const webm::ElementMetadata& metadata,
                      const webm::TrackEntry& track_entry) override {
    
		if (track_entry.track_type.is_present())
		{
			std::string codec = GetValue(track_entry.codec_id);

			if (codec != "V_VP9")
			{
				throw std::runtime_error("Unsupported codec");
			}

			switch(track_entry.track_type.value())
			{
				case webm::TrackType::kVideo:

					ReadVideoEntry(GetEntry(track_entry.video));
					break;
				default:
					throw std::runtime_error("Unsupported track entry");
			}
		}

    return webm::Status(webm::Status::kOkCompleted);
  }

	webm::Status OnFrame(const webm::FrameMetadata& metadata, webm::Reader* reader, std::uint64_t* bytes_remaining) override
	{
		_video.frameMetas.push_back(metadata);

		if (*bytes_remaining > _video.frameMaxSize)
		{
			_video.frameMaxSize = *bytes_remaining;
		}

		return webm::Callback::OnFrame(metadata, reader, bytes_remaining);
	}

	void ReadVideoEntry(const webm::Video& video)
	{
		if (video.interlaced.is_present() ||
				video.projection.is_present() ||
				video.alpha_mode.is_present() ||
				video.frame_rate.is_present())
		{
			throw std::runtime_error("Unsupported video property");
		}

		_video.width = GetValue(video.pixel_width);
		_video.height = GetValue(video.pixel_height);
	}

	Video& _video;
};

class Reader : public webm::Reader
{
public:

	Reader(data::Assets* assets, data::AssetHandle videoFileHandle) :
		_assets(assets),
		_videoFileHandle(videoFileHandle)
	{
	}

	webm::Status Read(std::size_t size, std::uint8_t* buffer, std::uint64_t* bytesRead) override
	{
		*bytesRead = _assets->ReadBytes(_videoFileHandle, buffer, size);

		if (*bytesRead == 0)
		{
			return webm::Status(webm::Status::kEndOfFile);
		}

		return webm::Status(*bytesRead < size ? webm::Status::kOkPartial : webm::Status::kOkCompleted);
	}

	webm::Status Skip(std::uint64_t bytesToSkip, std::uint64_t* actuallySkipped) override
	{
		int lastPosition = _assets->GetPosition(_videoFileHandle);

		_assets->Seek(_videoFileHandle, bytesToSkip, filesystem::FileSeekDir::Cur);

		*actuallySkipped = _assets->GetPosition(_videoFileHandle) - lastPosition;

		if (*actuallySkipped == 0)
		{
			return webm::Status(webm::Status::kEndOfFile);
		}

		return webm::Status(*actuallySkipped < bytesToSkip ? webm::Status::kOkPartial : webm::Status::kOkCompleted);
	}

  std::uint64_t Position() const override
	{
		return _assets->GetPosition(_videoFileHandle);
	}

private:

	data::Assets* _assets;
	data::AssetHandle _videoFileHandle;
};

int vpx_img_plane_width(const vpx_image_t *img, int plane) {
  if (plane > 0 && img->x_chroma_shift > 0)
    return (img->d_w + 1) >> img->x_chroma_shift;
  else
    return img->d_w;
}

int vpx_img_plane_height(const vpx_image_t *img, int plane) {
  if (plane > 0 && img->y_chroma_shift > 0)
    return (img->d_h + 1) >> img->y_chroma_shift;
  else
    return img->d_h;
}

// Unit transmission test
int main(int argc, char *argv[])
{
	using filesystem::Storage;
	using data::Assets;

	if (argc < 2)
	{
		std::cout << "Not enought arguments" << std::endl;
		return 1;
	}

	std::srand(std::time(nullptr));

	auto storagePath = argv[1];
	Storage storage(storagePath);
	Assets assets(&storage);

	while(true)
	{
		char filePath[260];
		data::AssetHandle videoAsset = nullptr;

		while(videoAsset == nullptr)
		{
			std::cout << "Write file path: " << std::endl;
			std::cin >> filePath;

			if (strcmp(filePath, "q") == 0)
				return 0;

			videoAsset = assets.Open(filePath);

			if (videoAsset == nullptr)
			{
				std::cout << "Failed to open file " << filePath << std::endl;
			}
		}

		Video            video;
		VideoCallback    callback(video);
		Reader           reader(&assets, videoAsset);
		webm::WebmParser parser;
		webm::Status     status = parser.Feed(&callback, &reader);

		if (status.completed_ok()) {
			std::cout << "Parsing successfully completed\n";
		} else {
			std::cout << "Parsing failed with status code: " << status.code << '\n';
		}

		vpx_codec_ctx_t codec;

		if (vpx_codec_dec_init(&codec, vpx_codec_vp9_dx(), nullptr, 0))
		{
			throw std::runtime_error("Failed to initialize VP9 decoder");
		}

		const int pixelSize = 3;

		auto byteCache = std::make_shared<uint8_t[]>(video.frameMaxSize);
		auto pixels    = std::make_shared<uint8_t[]>(video.width * video.height * pixelSize);

		int index = 0;

		for(auto frameMeta : video.frameMetas)
		{
			assets.Seek(videoAsset, frameMeta.position, filesystem::FileSeekDir::Beg);
			assets.ReadBytes(videoAsset, byteCache.get(), frameMeta.size);

			if (vpx_codec_decode(&codec, byteCache.get(), frameMeta.size, nullptr, 0))
			{
				throw std::runtime_error("Failed to decode frame");
			}

			vpx_codec_iter_t iter = nullptr;
			vpx_image_t      *img = nullptr;

			memset(pixels.get(), 0, video.width * video.height * pixelSize);

			while ((img = vpx_codec_get_frame(&codec, &iter)) != nullptr)
			{
				if (img->cs != VPX_CS_SRGB)
				{
					throw std::runtime_error("Unsupported color space");
				}

				if (img->fmt & VPX_IMG_FMT_HIGHBITDEPTH)
				{
					throw std::runtime_error("Unsupported depth format");
				}

				// Color order (G B R)
				const std::array<int, 3> channelMap = { 1, 2, 0 };

				for(int i = 0; i < 3; i++)
				{
					const uint8_t* buf = img->planes[i];
					const int stride = img->stride[i];

					int w = vpx_img_plane_width(img, i);
					int h = vpx_img_plane_height(img, i);

					// Assuming that for nv12 we write all chroma data at once
					if (img->fmt == VPX_IMG_FMT_NV12 && i > 1) break;
					// Fixing NV12 chroma width if it is odd
					if (img->fmt == VPX_IMG_FMT_NV12 && i == 1) w = (w + 1) & ~1;

					for (int y = 0; y < h; y++)
					{
						for (int x = 0; x < w; x++)
						{
							int pixelIndex = x + y * w;

							pixels[pixelIndex * pixelSize + channelMap[i]] = buf[x];
						};

						buf += stride;
					}
				}
			}

			std::stringstream nameStream;
			nameStream << "frames/" << index++ << ".png";

			DumpImageRGB(nameStream.str().c_str(), pixels.get(), video.width, video.height);
		}

		if (vpx_codec_destroy(&codec))
		{
			throw std::runtime_error("Failed to destroy VP9 codec context");
		}

		assets.Close(videoAsset);
	}

	return 0;
}