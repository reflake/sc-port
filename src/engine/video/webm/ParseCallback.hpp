#pragma once

#include <video/Video.hpp>
#include <stdexcept>

#include <webm/dom_types.h>
#include <webm/element.h>
#include <webm/callback.h>
#include <webm/status.h>

namespace video
{
	class ParseCallback : public webm::Callback
	{
	public:

		ParseCallback(VideoAsset& video) :
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

		
		webm::Status OnInfo(const webm::ElementMetadata& metadata, const webm::Info& info) override;

		webm::Status OnTrackEntry(const webm::ElementMetadata& metadata,
															const webm::TrackEntry& track_entry) override;

		webm::Status OnFrame(const webm::FrameMetadata& metadata, 
												webm::Reader* reader, uint64_t* bytes_remaining) override;

		void ReadVideoEntry(const webm::Video& video);

		VideoAsset& _video;
	};
}