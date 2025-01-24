#pragma once

#include "data/Assets.hpp"

#include <SDL2/SDL_mixer.h>
#include <SDL_rwops.h>
#include <memory>

namespace audio
{
	class MusicPlayer
	{
	public:

		MusicPlayer();
		MusicPlayer(data::Assets*);

		~MusicPlayer();

		void Initialize();
		void Play();
		void Stop(int msFadeTime);
		void Stop();
		void Process();
		void Release();

	private:

		void       PlayTrack(int index);
		void       ReadChunk();
		void       FreeChunk();

		int               _currentTrackIndex = 0;
		data::Assets*     _assets = nullptr;
		data::AssetHandle _openedMusicAsset = nullptr;

		std::shared_ptr<uint8_t[]> _musicData = nullptr;
		SDL_RWops* _readWriteOps = nullptr;
		Mix_Music *_musicChunk = nullptr;
	};
}