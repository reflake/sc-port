#pragma once

#include "meta/UnitTable.hpp"
#include <cstdint>
#include <utility>

namespace view
{
	enum SoundType
	{
		Death,
		TalkWhat, TalkPissed, TalkYes, TalkReady
	};

	const uint32_t SOUND_UNDEFINED = 0;

	class UnitSoundProfile
	{
	public:

		UnitSoundProfile(meta::UnitTable*);

		uint32_t TryTakeRandomAudio(uint32_t unitID, SoundType, uint32_t ignoredAudioId = SOUND_UNDEFINED);
		uint32_t TryTakeSequenceAudio(uint32_t unitID, SoundType, uint32_t previousAudioId = SOUND_UNDEFINED);

	private:

		inline std::pair<uint32_t, uint32_t> RangeOfAudioClips(uint32_t unitID, SoundType soundType)
		{
			switch(soundType)
			{
				case TalkYes:

					return { _unitTable->yesSoundStart[unitID], _unitTable->yesSoundEnd[unitID] };

				case TalkWhat:

					return { _unitTable->whatSoundStart[unitID], _unitTable->whatSoundEnd[unitID] };

				case TalkPissed:

					return { _unitTable->pissedSoundStart[unitID], _unitTable->pissedSoundEnd[unitID] };

				default:
					
					throw std::runtime_error("Not implemented yet!");
			}
		}

		const meta::UnitTable* _unitTable;
	};
};