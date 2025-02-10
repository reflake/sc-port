#pragma once

#include <cstdint>
namespace audio
{
	struct WaveHeader
	{
		char     magicRiff[4] = { 'R', 'I', 'F', 'F' }; // RIFF
		uint32_t restSize;
		char     magicWave[4] = { 'W', 'A', 'V', 'E' }; // WAVE
		char     magicFmt[4]  = { 'f', 'm', 't', ' ' };
		uint32_t formatLength;
		uint16_t format; // 1 - PCM
		uint16_t numOfChannels;
		uint32_t sampleRate;
		uint32_t byteRate;
		uint16_t blockAlign;
		uint16_t bitsPerSample;
		char     data[4] = { 'd', 'a', 't', 'a' };
		uint32_t dataSize;

		inline double GetDuration()
		{
			return static_cast<double>(dataSize) / byteRate;
		}
	};

	static_assert(sizeof(WaveHeader) == 44);
}