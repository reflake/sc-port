#include "UnitSoundProfile.hpp"
#include <stdexcept>
#include <experimental/random>

namespace view
{
	UnitSoundProfile::UnitSoundProfile(meta::UnitTable* unitTable)
		: _unitTable(unitTable)
	{}

	
	uint32_t UnitSoundProfile::TryTakeRandomAudio(uint32_t unitID, SoundType soundType, uint32_t ignoredAudioClip)
	{
		auto [soundStartIndex, soundEndIndex] = RangeOfAudioClips(unitID, soundType);

		if (soundStartIndex == 0)
			return SOUND_UNDEFINED;

		int audioClipId;
		int audioClipCountInRange = soundEndIndex - soundStartIndex + 1;

		if (audioClipCountInRange > 1 && (ignoredAudioClip < soundStartIndex || soundEndIndex < ignoredAudioClip || audioClipCountInRange == 2))
		{
			audioClipId = std::experimental::randint(soundStartIndex, soundEndIndex);
		}
		else if (audioClipCountInRange > 1)
		{
			// Do not play previous audio, only for voice line
			audioClipId = std::experimental::randint(0, audioClipCountInRange - 2) + 1;
			audioClipId = ignoredAudioClip + audioClipId;

			if (audioClipId > soundEndIndex)
			{
				audioClipId -= audioClipCountInRange;
			}
		}
		else 
		{
			audioClipId = soundStartIndex;
		}

		return audioClipId;
	}

	uint32_t UnitSoundProfile::TryTakeSequenceAudio(uint32_t unitID, SoundType soundType, uint32_t previousAudioId)
	{
		auto [soundStartIndex, soundEndIndex] = RangeOfAudioClips(unitID, soundType);

		if (soundStartIndex == 0)
			return SOUND_UNDEFINED;

		int audioClipId;
		int audioClipCountInRange = soundEndIndex - soundStartIndex + 1;

		if (audioClipCountInRange > 1)
		{
			// When cycle through for pissed voice lines
			if (previousAudioId < soundStartIndex || soundEndIndex < previousAudioId)
			{
				audioClipId = soundStartIndex;
			}
			else
			{
				// Take next line for pissed voice lines
				audioClipId = previousAudioId + 1;

				if (audioClipId > soundEndIndex)
				{
					audioClipId -= audioClipCountInRange;
				}
			}
		}
		else 
		{
			audioClipId = soundStartIndex;
		}
		
		return audioClipId;
	}
};