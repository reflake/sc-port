#include "AudioManager.hpp"

#include <SDL_mixer.h>
#include <SDL_rwops.h>
#include <cstdlib>
#include <stdexcept>

#include "data/Assets.hpp"
#include "Wave.hpp"

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

		for(auto& [k, v] : _channelSounds)
		{
			auto [mixChunk, fileRwOps, asset, _] = v;

			Mix_FreeChunk(mixChunk);
			SDL_FreeRW(fileRwOps);

			_assets->Close(asset);
		}
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

		_sfxTable       = _assets->Get<meta::SfxTable>("arr/sfxdata.dat");
		_sfxPathStrings = _assets->Get<data::StringsTable>("arr/sfxdata.tbl");
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


	int AudioManager::PlaySound(uint32_t index)
	{
		int pathIndex = _sfxTable->sfx[index];

		std::string filePath = std::string("SD/sound/") + _sfxPathStrings->entries[pathIndex];

		data::AssetHandle asset = _assets->Open(filePath.c_str());
		WaveHeader waveHeader;

		_assets->ReadBytes(asset, &waveHeader, sizeof(waveHeader));
		_assets->Seek(asset, 0, filesystem::FileSeekDir::Beg);

		assert(asset != nullptr);

		auto rwOps = SDL_AllocRW();
		_assets->AssetToSdlReadIO(rwOps, asset);
		
		auto mixChunk = Mix_LoadWAV_RW(rwOps, 0);

		int channel = Mix_PlayChannel(-1, mixChunk, 0);

		if (_channelSounds.contains(channel))
		{
			auto [mixChunk, fileRwOps, asset, _] = _channelSounds[channel];

			Mix_FreeChunk(mixChunk);
			SDL_FreeRW(fileRwOps);

			_assets->Close(asset);
		}

		_channelSounds[channel] = { mixChunk, rwOps, asset, waveHeader.GetDuration() };

		return channel;
	}

	bool AudioManager::IsSoundPlaying(int channel)
	{
		return Mix_Playing(channel);
	}

	double AudioManager::GetSoundDuration(int channel)
	{
		return _channelSounds[channel].duration;
	}

	void AudioManager::StopSoundInChannel(int channel)
	{
		Mix_HaltChannel(channel);
	}
};