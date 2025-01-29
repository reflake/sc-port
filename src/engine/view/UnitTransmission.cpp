#include "UnitTransmission.hpp"
#include "A_Graphics.hpp"
#include "data/TextStrings.hpp"
#include "meta/PortraitTable.hpp"
#include "video/Video.hpp"
#include <cstdlib>
#include <type_traits>
#include <experimental/random>

namespace view
{
	const int pixelSize = 4;

	UnitTransmission::UnitTransmission(data::Assets* assets, video::VideoManager* videoManager, renderer::A_Graphics* graphics, uint32_t width, uint32_t height) :
		_assets(assets),
		_graphics(graphics),
		_videoManager(videoManager),
		_width(width), _height(height)
	{
		_unitTable = _assets->ReadPreloaded<meta::UnitTable>("arr/units.dat");
		_portraitTable = _assets->ReadPreloaded<meta::PortraitTable>("arr/portdata.dat");
		_portraitPathStrings = _assets->ReadPreloaded<data::TextStringsTable>("arr/portdata.tbl");
	}

	void UnitTransmission::Draw(data::position pos)
	{
		if (_portraitId != -1 && _frameGraphicsHandle != nullptr)
		{
			_graphics->Draw(_frameGraphicsHandle, pos, _width, _height);
		}
	}

	void UnitTransmission::SetUnit(uint32_t unitId)
	{
		if (!_unitTable->portrait.HasElement(unitId))
		{
			_portraitId = -1;
			return;
		}

		_portraitId = _unitTable->portrait[unitId];

		uint32_t fidgetPathIndex = _portraitTable->fidgetStringPtr[_portraitId] - 1;
		_fidgetExtraChance = _portraitTable->fidgetChange[_portraitId];
		_fidgetClipCount = ReadClips(_fidgetClips, fidgetPathIndex);

		uint32_t talkingPathIndex = _portraitTable->talkingStringPtr[_portraitId] - 1;
		_talkingExtraChance = _portraitTable->talkingChange[_portraitId];
		_talkingClipCount = ReadClips(_talkingClips, talkingPathIndex);

		_hasTalkAnimation = fidgetPathIndex != talkingPathIndex;
		_isTalking = false;
		_currentClip = nullptr;
	}

	void UnitTransmission::Fidget()
	{
		if (_portraitId == -1)
			return;

		_isTalking = false;

		PickRandomClip(_fidgetClips, _fidgetClipCount);
	}

	void UnitTransmission::StartTalk(TalkType talkType)
	{
		if (!_hasTalkAnimation || _portraitId == -1)
			return;

		_talkTimer = 1.5;
		_isTalking = true;

		PickRandomClip(_talkingClips, _talkingClipCount);
	}

	// Call before Graphics.PresentToScreen()
	void UnitTransmission::Process(double deltaTime)
	{
		if (_currentClip == nullptr || _portraitId == -1)
			return;

		_nextFrameTimer -= deltaTime;
		_talkTimer -= deltaTime;

		if (_frameIterator == _currentClip->frames.begin())
		{
			// Recreate decoder
			if (_videoDecoder != nullptr)
			{
				delete _videoDecoder;
			}

			_videoDecoder = new video::Decoder(video::CodecType::VP9);
			_videoDecoder->Initialize();

			// Prepare first frame for encoding
			uint64_t maxFrameSize = std::max_element(_currentClip->frames.begin(), _currentClip->frames.end(), 
																					[] (auto& a, auto& b) { return a.size < b.size; })->size;

			_encodedPixels = std::make_shared<uint8_t[]>(maxFrameSize);

			// Create new frame pixels if needed
			if (_currentClip->width != _frameWidth || _currentClip->height != _frameHeight)
			{
				_frameWidth = _currentClip->width;
				_frameHeight = _currentClip->height;

				_framePixels = std::make_shared<uint8_t[]>(_frameWidth * _frameHeight * pixelSize);
				memset(_framePixels.get(), 255, _frameWidth * _frameHeight * pixelSize);
			}
		}

		// Need to play next frame
		if (_nextFrameTimer <= 0.0001)
		{
			auto& frame = *_frameIterator;

			_videoManager->ReadFrameData(_currentClip, frame, _encodedPixels.get());
			_videoDecoder->DecodeFrame(frame.size, _encodedPixels.get(), _framePixels.get());

			// Free previous frame
			if (_frameGraphicsHandle)
			{
				_graphics->FreeDrawable(_frameGraphicsHandle);
				_frameGraphicsHandle = nullptr;
			}

			_frameGraphicsHandle = _graphics->LoadImage(reinterpret_cast<uint32_t*>(_framePixels.get()), _frameWidth, _frameHeight);

			_nextFrameTimer = 1.0 / _currentClip->GetFPS();

			_frameIterator++;
		}
			
		bool lastFrame = _frameIterator == _currentClip->frames.end();

		if (lastFrame)
		{
			if (_isTalking && _talkTimer < 0.0)
			{
				Fidget();
			}

			// Loop the video and pick random clip
			PickRandomClip(_isTalking ? _talkingClips : _fidgetClips, 
										 _isTalking ? _talkingClipCount : _fidgetClipCount);

			_frameIterator = _currentClip->frames.begin();

		}
	}

	void UnitTransmission::PickRandomClip(PortraitClipArray& clips, int clipCount)
	{
		int index = 0;

		if (clipCount > 1)
		{
			int randomChange = std::rand() % 100;
			int changeChance = _isTalking ? _talkingExtraChance : _fidgetExtraChance;

			if (randomChange >= changeChance) // Pick random extra animations
			{
				index = std::experimental::randint(1, clipCount - 1);
			}
		}

		_frameIterator = clips[index].frames.begin();
		_currentClip = &clips[index];
	}

	int UnitTransmission::ReadClips(PortraitClipArray& clips, int pathIndex)
	{
		int i;

		for(i = 0; i < PORTRAIT_MAX_CLIPS; i++)
		{
			const char* filePath = _portraitPathStrings->entries[pathIndex];
			std::string fullPath = (boost::format("SD\\portrait\\%s%i.webm") % filePath % i).str();

			_videoManager->FreeVideo(&clips[i]);

			if (!_videoManager->OpenVideo(fullPath.c_str(), &clips[i]))
			{
				break;
			}
		}

		return i;
	}

	UnitTransmission::~UnitTransmission()
	{
		if (_frameGraphicsHandle != nullptr)
		{
			_graphics->FreeDrawable(_frameGraphicsHandle);
		}

		for(int i = 0; i < _fidgetClipCount; i++)
		{
			_videoManager->FreeVideo(&_fidgetClips[i]);
		}

		for(int i = 0; i < _talkingClipCount; i++)
		{
			_videoManager->FreeVideo(&_talkingClips[i]);
		}

		if (_videoDecoder != nullptr)
		{
			delete _videoDecoder;
		}
	}
}