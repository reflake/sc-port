#include "Music.hpp"
#include <SDL_mixer.h>
#include <SDL_rwops.h>
#include <array>
#include <cassert>
#include <cstdlib>
#include <memory>
#include <stdexcept>
#include <utility>

using std::runtime_error;

namespace audio
{
	const int chunkSize = 10000;

	std::array<const char*, 4> tracks = {
		"music/terran1.ogg",
		"music/terran2.ogg",
		"music/terran3.ogg",
		"music/terran4.ogg",
	};

	MusicPlayer::MusicPlayer()
		{}
		
	MusicPlayer::MusicPlayer(data::Assets* assets)
		: _assets(assets)
		{}

	void MusicPlayer::Initialize()
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

	void MusicPlayer::Play()
	{
		FreeChunk();
		PlayTrack(std::rand() % tracks.size());
	}

	void MusicPlayer::Stop(int millisecondsFadeTime)
	{
		Mix_FadeOutMusic(millisecondsFadeTime);
	}

	void MusicPlayer::Stop()
	{
		Mix_HaltMusic();

		FreeChunk();
	}

	void MusicPlayer::Process()
	{
		if (!Mix_PlayingMusic())
		{
			FreeChunk();

			// Music ended
			if (_musicChunk == nullptr)
			{
				int random = std::rand() % (tracks.size() - 1);
				random = (_currentTrackIndex + random) % tracks.size();

				PlayTrack(random);
			}
		}
	}

	void MusicPlayer::PlayTrack(int track)
	{
		_currentTrackIndex = track;

		if (_openedMusicAsset != nullptr)
		{
			_assets->Close(_openedMusicAsset);
		}

		_openedMusicAsset = _assets->Open(tracks[track]);

		ReadChunk();

		assert(_musicChunk != nullptr);

		Mix_PlayMusic(_musicChunk, 0);
	}

	void MusicPlayer::ReadChunk()
	{
		assert(_openedMusicAsset != nullptr);

		_readWriteOps = SDL_AllocRW();
		_assets->AssetToSdlReadIO(_readWriteOps, _openedMusicAsset);

		_musicChunk = Mix_LoadMUSType_RW(_readWriteOps, MUS_OGG, 0);
	}

	void MusicPlayer::FreeChunk()
	{
		if (_musicChunk != nullptr)
			Mix_FreeMusic(_musicChunk);

		if (_readWriteOps != nullptr)
			SDL_FreeRW(_readWriteOps);

		_musicChunk   = nullptr;
		_readWriteOps = nullptr;
	}

	MusicPlayer::~MusicPlayer()
	{
		FreeChunk();
	}
}