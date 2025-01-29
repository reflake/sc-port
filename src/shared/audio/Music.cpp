#include "Music.hpp"
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
		
	MusicPlayer::MusicPlayer(AudioManager* audioManager) :
		_audioManager(audioManager)
	{}

	void MusicPlayer::Initialize()
	{
	}

	void MusicPlayer::Play()
	{
		FreeMusic();
		PlayTrack(std::rand() % tracks.size());
	}

	void MusicPlayer::Stop(int millisecondsFadeTime)
	{
		_audioManager->FadeMusic(millisecondsFadeTime);
	}

	void MusicPlayer::Stop()
	{
		_audioManager->HaltMusic();

		FreeMusic();
	}

	void MusicPlayer::Process()
	{
		if (!_audioManager->IsMusicPlaying())
		{
			FreeMusic();

			// Music ended
			int random = std::rand() % (tracks.size() - 1);
			random = (_currentTrackIndex + random) % tracks.size();

			PlayTrack(random);
		}
	}

	void MusicPlayer::PlayTrack(int trackIndex)
	{
		_currentTrackIndex = trackIndex;

		_audioManager->PlayMusic(tracks[trackIndex]);
	}

	void MusicPlayer::FreeMusic()
	{
		_audioManager->FreeMusic();
	}

	MusicPlayer::~MusicPlayer()
	{
		_audioManager->FreeMusic();
	}
}