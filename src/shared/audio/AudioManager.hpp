#pragma once

#include "data/Assets.hpp"
#include <SDL2/SDL_mixer.h>
#include <SDL_mixer.h>
#include <SDL_rwops.h>
#include <SDL_rwops.h>

namespace audio
{
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

		bool IsMusicPlaying();

	private:

		data::Assets* _assets = nullptr;

		SDL_RWops* _musicReadWriteOps = nullptr;
		Mix_Music *_music = nullptr;

		data::AssetHandle _openedMusicAsset = nullptr;
	};
}