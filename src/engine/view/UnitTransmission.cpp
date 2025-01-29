#include "UnitTransmission.hpp"
#include "A_Graphics.hpp"
#include "audio/AudioManager.hpp"
#include "data/TextStrings.hpp"
#include "meta/PortraitTable.hpp"
#include "video/Video.hpp"
#include <cstdlib>
#include <type_traits>
#include <experimental/random>

namespace view
{
	const int pixelSize = 4;

	using namespace data;
	using namespace meta;

	UnitTransmission::UnitTransmission(
		Assets* assets, video::VideoManager* videoManager, renderer::A_Graphics* graphics, 
		audio::AudioManager* audioManager, uint32_t width, uint32_t height) :

		_assets(assets),
		_graphics(graphics),
		_videoManager(videoManager),
		_audioManager(audioManager),
		_width(width), _height(height)
	{
		_portraitTable       = _assets->Get<PortraitTable>("arr/portdata.dat");
		_portraitPathStrings = _assets->Get<StringsTable>("arr/portdata.tbl");
		_unitTable           = _assets->Get<UnitTable>("arr/units.dat");
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

		_unitId = unitId;
		_portraitId = _unitTable->portrait[unitId];

		uint32_t fidgetPathIndex = _portraitTable->fidgetStringPtr[_portraitId] - 1;
		_fidgetClipCount = ReadClips(_fidgetClips, fidgetPathIndex);

		uint32_t talkingPathIndex = _portraitTable->talkingStringPtr[_portraitId] - 1;
		_talkingClipCount = ReadClips(_talkingClips, talkingPathIndex);

		_hasTalkAnimation = fidgetPathIndex != talkingPathIndex;
		_isTalking = false;
		_currentClip = nullptr;

		if (_soundChannel != -1)
			_audioManager->StopSoundInChannel(_soundChannel);

		_soundChannel = -1;
		_lastVoiceLinePlayed = -1;
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
		// if already is talking then ignore
		if (!_hasTalkAnimation || _portraitId == -1)
			return;

		if (_unitId == -1)
			return;

		if (_audioManager == nullptr)
			return;

		// If talking then skip
		if (IsSoundPlaying())
			return;

		int soundStartIndex, soundEndIndex;

		switch(talkType)
		{
			case TalkYes:

				soundStartIndex = _unitTable->yesSoundStart[_unitId];
				soundEndIndex = _unitTable->yesSoundEnd[_unitId];
				break;

			case TalkWhat:

				soundStartIndex = _unitTable->whatSoundStart[_unitId];
				soundEndIndex = _unitTable->whatSoundEnd[_unitId];
				break;

			case TalkPissed:

				soundStartIndex = _unitTable->pissedSoundStart[_unitId];
				soundEndIndex = _unitTable->pissedSoundEnd[_unitId];
				break;
		}

		if (soundStartIndex == 0)
			return;

		int soundIndex;
		int voiceLineCount = soundEndIndex - soundStartIndex + 1;

		if (voiceLineCount > 1 && talkType != TalkPissed && (_lastVoiceLinePlayed < soundStartIndex || soundEndIndex < _lastVoiceLinePlayed || voiceLineCount == 2))
		{
			soundIndex = std::experimental::randint(soundStartIndex, soundEndIndex);
		}
		else if (voiceLineCount > 1 && talkType != TalkPissed)
		{
			// Do not play previous voice line
			soundIndex = std::experimental::randint(0, voiceLineCount - 2) + 1;
			soundIndex = _lastVoiceLinePlayed + soundIndex;

			if (soundIndex > soundEndIndex)
			{
				soundIndex -= voiceLineCount;
			}
		}
		else if (voiceLineCount > 1)
		{
			// When pissed cycle through voice lines
			if (_lastVoiceLinePlayed < soundStartIndex || soundEndIndex < _lastVoiceLinePlayed)
			{
				soundIndex = soundStartIndex;
			}
			else
			{
				soundIndex = ++_lastVoiceLinePlayed;

				if (soundIndex > soundEndIndex)
				{
					soundIndex -= voiceLineCount;
				}
			}
		}
		else 
		{
			soundIndex = soundStartIndex;
		}
		
		_soundChannel = _audioManager->PlaySound(soundIndex - 1);
		_lastVoiceLinePlayed = soundIndex;

		_isTalking = true;

		double soundDuration = _audioManager->GetSoundDuration(_soundChannel);
		double durationFraction;
		double durationIntegral;

		durationFraction = std::modf(soundDuration, &durationIntegral);

		if (durationFraction < 0.2 || 0.8 < durationFraction)
		{
			_talkingAnimationTimer = durationIntegral + std::round(durationFraction * 10.0) / 10.0;
		}
		else
		{
			_talkingAnimationTimer = std::round(soundDuration);
			_talkingAnimationTimer = std::max(1.0, _talkingAnimationTimer);
		}

		PickRandomClip(_talkingClips, _talkingClipCount);
	}

	// Call before Graphics.PresentToScreen()
	void UnitTransmission::Process(double deltaTime)
	{
		if (_currentClip == nullptr || _portraitId == -1)
			return;

		_nextFrameTimer -= deltaTime;
		_talkingAnimationTimer -= deltaTime;

		if (_frameIterator == _currentClip->frames.begin())
		{
			// Recreate decoder
			if (_videoDecoder != nullptr)
			{
				delete _videoDecoder;
			}

			_videoDecoder = new video::Decoder(video::CodecType::VP9);
			_videoDecoder->Initialize();
		}

		// Need to play next frame
		if (_nextFrameTimer <= 0.0001 && _currentClip != nullptr)
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

		if (_isTalking && _talkingAnimationTimer < 0)
		{
			Fidget();
		}
			
		bool lastFrame = _frameIterator == _currentClip->frames.end();

		if (lastFrame)
		{
			// Loop the video and pick random clip
			PickRandomClip(_isTalking ? _talkingClips : _fidgetClips, 
										 _isTalking ? _talkingClipCount : _fidgetClipCount);

			_frameIterator = _currentClip->frames.begin();

		}
	}

	bool UnitTransmission::IsSoundPlaying()
	{
		return _audioManager->IsSoundPlaying(_soundChannel);	
	}

	void UnitTransmission::PickRandomClip(PortraitClipArray& clips, int clipCount)
	{
		int index = 0;

		if (clipCount > 1)
		{
			int randomChange = std::rand() % 100;
			int changeChance = _isTalking ? _portraitTable->talkingChange[_portraitId] : _portraitTable->fidgetChange[_portraitId];

			if (randomChange >= changeChance) // Pick random extra animations
			{
				index = std::experimental::randint(1, clipCount - 1);
			}
		}

		_frameIterator = clips[index].frames.begin();
		_currentClip = &clips[index];

		// Create new frame pixels if needed
		if (_currentClip->width != _frameWidth || _currentClip->height != _frameHeight)
		{
			_frameWidth = _currentClip->width;
			_frameHeight = _currentClip->height;

			_framePixels = std::make_shared<uint8_t[]>(_frameWidth * _frameHeight * pixelSize);
			memset(_framePixels.get(), 255, _frameWidth * _frameHeight * pixelSize);
		}

		// Prepare frame buffer for decoding
		uint64_t maxFrameSize = std::max_element(_currentClip->frames.begin(), _currentClip->frames.end(), 
																				[] (auto& a, auto& b) { return a.size < b.size; })->size;

		_encodedPixels = std::make_shared<uint8_t[]>(maxFrameSize);
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