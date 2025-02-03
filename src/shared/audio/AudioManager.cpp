#include "AudioManager.hpp"

#include <array>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <math.h>
#include <memory>
#include <stdexcept>

#include "SoundStream.hpp"
#include "data/Assets.hpp"
#include "Wave.hpp"
#include "diagnostic/Clock.hpp"

#include <AL/al.h>
#include <AL/alc.h>
#include <AL/efx-creative.h>
#include <vector>

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

	}

	void AudioManager::Initialize()
	{
		_device = alcOpenDevice(nullptr);

		if (_device == nullptr)
		{
			throw runtime_error("OpenAL: Failed to open audio device");
		}

		_context = alcCreateContext(_device, nullptr);

		if (_context == nullptr)
		{
			throw runtime_error("OpenAL: Failed to create context");
		}

		alcMakeContextCurrent(_context);

		bool eaxSupport = alcIsExtensionPresent(_device, "EAX2.0");

		alGetError();

		uint32_t buffers[32];

		alGenBuffers(32, buffers);

		if (alGetError() != AL_NO_ERROR)
		{
			throw runtime_error("OpenAL: Failed to generate buffers");
		}

		for(int i = 0; i < 32; i++)
		{
			_bufferPool.push_back(buffers[i]);
		}

		uint32_t sources[4];

		alGenSources(4, sources);

		if (alGetError() != AL_NO_ERROR)
		{
			throw runtime_error("OpenAL: Failed to generate sources");
		}

		for(int i = 0; i < 4; i++)
		{
			_sourcePool.push_back(sources[i]);
		}

		_sfxTable       = _assets->Get<meta::SfxTable>("arr/sfxdata.dat");
		_sfxPathStrings = _assets->Get<data::StringsTable>("arr/sfxdata.tbl");

		_copyBuffer = std::make_shared<uint8_t[]>(_bufferSize);
	}

	void AudioManager::PlayMusic(const char* path)
	{
		_openedMusicAsset = _assets->Open(path);
	}

	void AudioManager::FadeMusic(int millisecondsFadeTime)
	{
	}

	void AudioManager::HaltMusic()
	{
	}

	void AudioManager::FreeMusic()
	{
		if (_openedMusicAsset != nullptr)
			_assets->Close(_openedMusicAsset);

		_openedMusicAsset = nullptr;
	}

	void AudioManager::Process()
	{
		std::vector<int> soundsPlayed;

		for(auto& [index, sound] : _playbackSounds)
		{
			int queuedBufferPlayed, queuedBufferCount = 0;
			alGetSourcei(sound.source, AL_BUFFERS_PROCESSED, &queuedBufferPlayed);
			alGetSourcei(sound.source, AL_BUFFERS_QUEUED, &queuedBufferCount);

			int err = alGetError();

			switch (err) {
				case AL_NO_ERROR: break;
				case AL_INVALID_NAME: throw runtime_error("OpenAL: Failed to get source value, invalid source");
				default: throw runtime_error("OpenAL: Failed to get source value, reason unknown");
			}

			for(; queuedBufferPlayed; queuedBufferPlayed--)
			{
				uint32_t playedBuffer = sound.queuedBufferIndices[0];

				alSourceUnqueueBuffers(sound.source, 1, &playedBuffer);

				// Return buffer into pool
				_bufferPool.push_back(playedBuffer);

				std::swap(sound.queuedBufferIndices[0], sound.queuedBufferIndices[1]);

				if (queuedBufferPlayed == queuedBufferCount && sound.stream.IsEndOfStream())
				{
					soundsPlayed.push_back(index);
					continue;
				}

				assert(queuedBufferPlayed != 2);

				int err = alGetError();

				switch (err) {
					case AL_NO_ERROR: break;
					case AL_INVALID_NAME: throw runtime_error("OpenAL: Failed to unqueue buffers, invalid source");
					case AL_INVALID_VALUE: throw runtime_error("OpenAL: Failed to unqueue buffers, invalid buffer(s)");
					default: throw runtime_error("OpenAL: Failed to unqueue buffers, reason unknown");
				}

				uint32_t nextBuffer = ReadBufferData(sound.stream);

				if (nextBuffer != 0)
				{
					alSourceQueueBuffers(sound.source, 1, &nextBuffer);

					sound.queuedBufferIndices[1] = nextBuffer;

					int err = alGetError();

					switch (err) {
						case AL_NO_ERROR: break;
						case AL_INVALID_NAME: throw runtime_error("OpenAL: Failed to queue buffers, invalid source/buffer(s)");
						default: throw runtime_error("OpenAL: Failed to queue buffers, reason unknown");
					}
				}
			}
		}

		for(auto index : soundsPlayed)
		{
			DestroySound(index);
		}
	}

	bool AudioManager::IsMusicPlaying()
	{
		return false;
	}

	int AudioManager::PlaySound(uint32_t index)
	{
		Clock clock("PlaySound");

		int pathIndex = _sfxTable->sfx[index];

		std::string filePath = std::string("SD/sound/") + _sfxPathStrings->entries[pathIndex];

		data::AssetHandle soundAsset = _assets->Open(filePath.c_str());
		assert(soundAsset != nullptr);

		WaveHeader waveHeader;
		_assets->ReadBytes(soundAsset, &waveHeader, sizeof(waveHeader));

		assert(waveHeader.bitsPerSample == 16);
		assert(waveHeader.numOfChannels == 1);

		int size = waveHeader.dataSize;

		// Fill buffers with sound data
		std::array<uint32_t, 2> queuedBuffers;
		int                     queuedBufferCount = 0;
		SoundStream             soundStream(_assets, soundAsset, size);

		for(int i = 0; i < queuedBuffers.size(); i++)
		{
			uint32_t buffer = ReadBufferData(soundStream);

			if (buffer != 0)
			{
				queuedBuffers[i] = buffer;
				queuedBufferCount++;
			}
		}

		// Queue chosen buffers
		if (queuedBufferCount == 0)
			return -1;

		uint32_t source = _sourcePool.back();
		_sourcePool.pop_back();

		alSourceQueueBuffers(source, queuedBufferCount, queuedBuffers.data());

		int err = alGetError();

		switch (err) {
			case AL_NO_ERROR: break;
			case AL_INVALID_NAME: throw runtime_error("OpenAL: Failed to play source, invalid source/buffer(s)");
			default: throw runtime_error("OpenAL: Failed to play source, reason unknown");
		}

		alSourcePlay(source);

		err = alGetError();

		switch (err) {
			case AL_NO_ERROR: break;
			case AL_INVALID_NAME: throw runtime_error("OpenAL: Failed to play source, invalid source");
			default: throw runtime_error("OpenAL: Failed to play source, reason unknown");
		}

		// Create a new playback sound and give it "unique" id
		_playbackSounds[_soundIndex] = { source, std::move(soundStream), waveHeader.GetDuration(), queuedBuffers };

		return (_soundIndex += 10) - 10;
	}

	uint32_t AudioManager::ReadBufferData(SoundStream& stream)
	{
		if (stream.IsEndOfStream())
			return 0;

		int size = stream.ReadChunk(_bufferSize, _copyBuffer.get());

		if (_bufferPool.empty())
			return 0;

		uint32_t freeBuffer = _bufferPool.back();
		_bufferPool.pop_back();

		alBufferData(freeBuffer, AL_FORMAT_MONO16, _copyBuffer.get(), size, 22100);

		int err = alGetError();

		if (err != AL_NO_ERROR)
		{
			switch(err)
			{
				case AL_OUT_OF_MEMORY:
					throw runtime_error("OpenAL: Failed to copy data into a buffer, out of memory");
				case AL_INVALID_VALUE:
					throw runtime_error("OpenAL: Failed to copy data into a buffer, invalid argument");
				case AL_INVALID_ENUM:
					throw runtime_error("OpenAL: Failed to copy data into a buffer, invalid format");
				default:
					throw runtime_error("OpenAL: Failed to copy data into a buffer, reason unknown");
			}
		}

		return freeBuffer;
	}

	bool AudioManager::IsSoundPlaying(int soundId)
	{
		return _playbackSounds.contains(soundId);
	}

	double AudioManager::GetSoundDuration(int soundId)
	{
		return _playbackSounds.contains(soundId) ? _playbackSounds[soundId].duration : 0.0;
	}

	void AudioManager::StopSoundInChannel(int soundId)
	{
		if (_playbackSounds.contains(soundId))
		{
			uint32_t source = _playbackSounds[soundId].source;

			alSourceStop(source);
			alGetError(); // clear error

			DestroySound(soundId);
		}
	}

	void AudioManager::DestroySound(int soundId)
	{
		auto& sound = _playbackSounds[soundId];

		// Return source into pool
		_sourcePool.push_back(sound.source);

		// Free all buffers
		int bufferCount;
		alGetSourcei(sound.source, AL_BUFFERS_QUEUED, &bufferCount);
		alGetError(); // clear error

		std::array<uint32_t, 2> queueBuffers;

		for(int i = 0; i < bufferCount; i++)
		{
			auto& buffer = sound.queuedBufferIndices[i];

			queueBuffers[i] = buffer;

			// Return buffer into pool
			_bufferPool.push_back(buffer);
		}

		if (bufferCount > 0)
		{
			alSourceUnqueueBuffers(sound.source, bufferCount, queueBuffers.data());

			int err = alGetError();

			switch (err) {
				case AL_NO_ERROR: break;
				case AL_INVALID_NAME: throw runtime_error("OpenAL: Failed to unqueue buffers, invalid source");
				case AL_INVALID_VALUE: throw runtime_error("OpenAL: Failed to unqueue buffers, invalid buffer(s)");
				default: throw runtime_error("OpenAL: Failed to unqueue buffers, reason unknown");
			}
		}

		_playbackSounds.erase(soundId);
	}
};