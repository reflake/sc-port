#include "AudioManager.hpp"

#include <SDL_mixer.h>
#include <cstdlib>
#include <stdexcept>

namespace audio
{
	using std::runtime_error;

	AudioManager::AudioManager()
	{}
	
	AudioManager::AudioManager(data::Assets* assets) :
		_assets(assets)
	{}

	AudioManager::~AudioManager()
	{
		FreeMusic();
	}

	void AudioManager::Initialize()
	{
		if (Mix_Init(MIX_INIT_OGG) < 0)
		{
			throw runtime_error("Failed to initialize audio engine");
		}

		if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096) < 0)
		{
			throw runtime_error("Failed to open audio mixer");
		}
	}

	void AudioManager::PlayMusic(const char* path)
	{
		// Free previously played music
		FreeMusic();

		_openedMusicAsset = _assets->Open(path);

		assert(_openedMusicAsset != nullptr);

		_musicReadWriteOps = SDL_AllocRW();
		_assets->AssetToSdlReadIO(_musicReadWriteOps, _openedMusicAsset);

		_music = Mix_LoadMUSType_RW(_musicReadWriteOps, MUS_OGG, 0);

		assert(_music != nullptr);

		Mix_PlayMusic(_music, 0);
	}

	void AudioManager::FadeMusic(int millisecondsFadeTime)
	{
		Mix_FadeOutMusic(millisecondsFadeTime);
	}

	void AudioManager::HaltMusic()
	{
		Mix_HaltMusic();
	}

	void AudioManager::FreeMusic()
	{
		if (_music != nullptr)
			Mix_FreeMusic(_music);

		if (_musicReadWriteOps != nullptr)
			SDL_FreeRW(_musicReadWriteOps);

		if (_openedMusicAsset != nullptr)
			_assets->Close(_openedMusicAsset);

		_music = nullptr;
		_musicReadWriteOps = nullptr;
		_openedMusicAsset = nullptr;
	}

	bool AudioManager::IsMusicPlaying()
	{
		return Mix_PlayingMusic();
	}
};