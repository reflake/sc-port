#pragma once

#include "SoundStream.hpp"
#include "data/Assets.hpp"
#include "data/TextStrings.hpp"
#include "meta/SfxTable.hpp"

#include <AL/alc.h>
#include <memory>
#include <unordered_map>

namespace audio
{
	struct PlaybackSound
	{
		uint32_t           source;
		SoundStream        stream;
		double             duration;
		std::array<uint32_t, 2> queuedBufferIndices;
	};

	class AudioManager
	{
	public:

		AudioManager();
		AudioManager(data::Assets*);

		~AudioManager();

		void Initialize();

		void PlayMusic(const char* path);
		void FadeMusic(int millisecondsFadeTime);
		void HaltMusic();
		void FreeMusic();
		void Process();

		// Returns channel in which sound is being played
		int  PlaySound(uint32_t audioIndex);
		bool IsSoundPlaying(int soundId);
		double GetSoundDuration(int soundId);
		void StopSoundInChannel(int soundId);

		bool IsMusicPlaying();

		void Release();

	private:

		uint32_t ReadBufferData(SoundStream& stream);

		void DestroySound(int soundId);

		uint32_t _frequency = 44100;
		uint32_t _bufferSize = 4096;

		const meta::SfxTable*         _sfxTable;
		const data::StringsTable*     _sfxPathStrings;

		ALCdevice*  _device;
		ALCcontext* _context;

		data::Assets* _assets = nullptr;

		data::AssetHandle _openedMusicAsset = nullptr;

		std::unordered_map<int, PlaybackSound> _playbackSounds;

		int _soundIndex = 0;
		
		std::vector<uint32_t>      _bufferPool;
		std::vector<uint32_t>      _sourcePool;
		std::shared_ptr<uint8_t[]> _copyBuffer;
	};
}