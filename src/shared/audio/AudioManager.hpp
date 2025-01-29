#pragma once

#include "data/Assets.hpp"
#include "data/TextStrings.hpp"
#include "meta/SfxTable.hpp"

#include <SDL2/SDL_mixer.h>
#include <SDL_mixer.h>
#include <SDL_rwops.h>
#include <SDL_rwops.h>
#include <unordered_map>

namespace audio
{
	struct Sound
	{
		Mix_Chunk* chunk;
		SDL_RWops* fileRwOps;
		data::AssetHandle assetHandle;
		double duration;
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

		// Returns channel in which sound is being played
		int  PlaySound(uint32_t index);
		bool IsSoundPlaying(int channel);
		double GetSoundDuration(int channel);
		void StopSoundInChannel(int channel);

		bool IsMusicPlaying();

	private:

		const meta::SfxTable*         _sfxTable;
		const data::StringsTable*     _sfxPathStrings;

		data::Assets* _assets = nullptr;

		SDL_RWops* _musicReadWriteOps = nullptr;
		Mix_Music *_music = nullptr;

		data::AssetHandle _openedMusicAsset = nullptr;

		std::unordered_map<int, Sound> _channelSounds;
	};
}