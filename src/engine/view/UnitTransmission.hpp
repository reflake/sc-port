#pragma once

#include "A_Graphics.hpp"
#include "audio/AudioManager.hpp"
#include "data/Assets.hpp"
#include "data/Common.hpp"
#include "data/TextStrings.hpp"
#include "meta/SfxTable.hpp"
#include "meta/PortraitTable.hpp"
#include "meta/UnitTable.hpp"
#include "video/Decoder.hpp"
#include "video/Video.hpp"

namespace view
{
	enum TalkType
	{
		TalkWhat, TalkPissed, TalkYes, TalkReady
	};

	const int PORTRAIT_MAX_CLIPS = 4;

	typedef std::array<video::VideoAsset, PORTRAIT_MAX_CLIPS> PortraitClipArray;

	class UnitTransmission
	{
	public:

		UnitTransmission(data::Assets* assets, video::VideoManager*, renderer::A_Graphics*, 
											audio::AudioManager*, uint32_t width, uint32_t height);
		~UnitTransmission();

		void Draw(data::position);

		void SetUnit(uint32_t unitId);

		void Fidget();

		// Picks random quote of unit
		void StartTalk(TalkType);

		void Process(double deltaTime);

		bool IsSoundPlaying();

	private:

		void PickRandomClip(PortraitClipArray& clips, int clipCount);
		int  ReadClips(PortraitClipArray& clips, int pathIndex);

		renderer::DrawableHandle _currentFrameHandle = nullptr;

		audio::AudioManager*  _audioManager;
		video::VideoManager*  _videoManager;
		renderer::A_Graphics* _graphics;

		const data::Assets*           _assets;
		const meta::PortraitTable*    _portraitTable;
		const data::StringsTable*     _portraitPathStrings;
		const meta::UnitTable*        _unitTable;

		double   _talkingAnimationTimer = 0;
		int      _voiceSoundId = -1;
		bool     _isTalking = false;
		bool     _hasTalkAnimation = false;

		double   _nextFrameTimer = 0.0;
		int      _unitId = -1;
		int      _lastVoiceLinePlayed = -1;
		int      _portraitId;
		uint32_t _width, _height;

		int                              _fidgetClipCount, _talkingClipCount;
		PortraitClipArray                _fidgetClips, _talkingClips;
		std::shared_ptr<uint8_t[]>       _framePixels, _encodedPixels;
		uint32_t                         _frameWidth = 0, _frameHeight = 0;
		renderer::DrawableHandle         _frameGraphicsHandle = nullptr;

		video::VideoAsset* _currentClip  = nullptr;
		video::Decoder*    _videoDecoder = nullptr;

		std::vector<video::FrameMeta>::iterator _frameIterator;
	};
}