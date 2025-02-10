#pragma once

#include "AudioManager.hpp"
#include "data/Assets.hpp"

#include <memory>

namespace audio
{
	class MusicPlayer
	{
	public:

		MusicPlayer();
		MusicPlayer(AudioManager*);

		~MusicPlayer();

		void Initialize();
		void Play();
		void Stop(int msFadeTime);
		void Stop();
		void Process();
		void Release();

	private:

		void PlayTrack(int index);
		void FreeMusic();

	private:

		AudioManager* _audioManager = nullptr;
		int           _currentTrackIndex = 0;
	};
}