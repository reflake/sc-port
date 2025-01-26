#include "ParseCallback.hpp"
#include "video/Video.hpp"
#include <webm/dom_types.h>

namespace video
{
	using webm::Info;
	using webm::Status;
	using webm::FrameMetadata;
	using webm::ElementMetadata;
	using webm::Reader;
	using webm::TrackEntry;

	Status ParseCallback::OnInfo(const ElementMetadata& metadata, const Info& info)
	{
		_video.duration = GetValue(info.duration) / 1000.0;

		return Status(Status::Code::kOkCompleted);
	}

	Status ParseCallback::OnTrackEntry(const ElementMetadata& metadata,
																		 const TrackEntry& track_entry)
	{
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

		return Status(Status::kOkCompleted);
	}

	Status ParseCallback::OnFrame(const FrameMetadata& metadata, Reader* reader, uint64_t* bytes_remaining)
	{
		_video.frames.push_back({ metadata.position, metadata.size });

		return Callback::OnFrame(metadata, reader, bytes_remaining);
	}

	void ParseCallback::ReadVideoEntry(const webm::Video& video)
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
}