#include <cstdio>
#include <cstring>
#include <ctime>
#include <iostream>
#include <memory>
#include <sstream>

#include <vpx/vpx_codec.h>
#include <vpx/vpx_image.h>
#include <webm/id.h>
#include <webm/reader.h>
#include <webm/status.h>
#include <webm/file_reader.h>
#include <webm/webm_parser.h>
#include <webm/dom_types.h>

#include "data/Assets.hpp"
#include "diagnostic/Image.hpp"
#include "video/Decoder.hpp"
#include "video/Video.hpp"

#include <filesystem/Storage.hpp>

// Unit transmission test
int main(int argc, char *argv[])
{
	using filesystem::Storage;
	using data::Assets;
	using data::AssetHandle;

	if (argc < 2)
	{
		std::cout << "Not enought arguments" << std::endl;
		return 1;
	}

	std::srand(std::time(nullptr));

	auto storagePath = argv[1];
	Storage storage(storagePath);
	Assets  assets(&storage);

	video::VideoManager videoManager(&assets);

	while(true)
	{
		char filePath[260];

		video::VideoAsset video;

		while(video.assetHandle == nullptr)
		{
			std::cout << "Write file path: " << std::endl;
			std::cin >> filePath;

			if (strcmp(filePath, "q") == 0)
				return 0;

			if (videoManager.OpenVideo(filePath, &video))
			{
				std::cout << "Parsing successfully completed\n";
				std::cout << "Video fps: " << video.GetFPS() << "\n\n";
			} 
			else 
			{
				std::cout << "Failed to open file " << filePath << std::endl;
			}
		}

		video::Decoder videoDecoder(video::CodecType::VP9);

		videoDecoder.Initialize();

		uint64_t maxFrameSize = std::max_element(video.frames.begin(), video.frames.end(), 
																				[] (auto& a, auto& b) { return a.size < b.size; })->size;

		const int pixelSize = 3;
		auto encodedData = std::make_shared<uint8_t[]>(maxFrameSize);
		auto pixels      = std::make_shared<uint8_t[]>(video.width * video.height * pixelSize);

		int index = 0;

		for(auto frame : video.frames)
		{
			videoManager.ReadFrameData(&video, frame, encodedData.get());

			videoDecoder.DecodeFrame(frame.size, encodedData.get(), pixels.get());

			std::stringstream nameStream;
			nameStream << "frames/" << index++ << ".png";

			DumpImageRGB(nameStream.str().c_str(), pixels.get(), video.width, video.height);
		}

		videoManager.FreeVideo(&video);
	}

	return 0;
}