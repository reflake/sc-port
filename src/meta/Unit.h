#pragma once

#include <cstdint>
#include <glm/vec4.hpp>

#include "../filesystem/storage.hpp"

namespace meta
{
	typedef uint8_t direction;

	const int MAX_AMOUNT = 228;
	const int MAX_INFESTATION_AMOUNT = 96;
	const int MAX_PISSED_SOUND_AMOUNT = 106;
	const int MAX_YES_SOUND_AMOUNT = 106;
	const int MAX_READY_SOUND_AMOUNT = 106;
	const int MAX_ADDON_AMOUNT = 96;

	typedef glm::vec<4, uint16_t> unitDimensions;

	#pragma pack (push, 1)
	struct UnitTable
	{
		uint8_t flingyID[MAX_AMOUNT];

		uint16_t subunitIDs[2][MAX_AMOUNT];

		uint16_t infestationID[MAX_INFESTATION_AMOUNT];

		uint32_t constructionAnimation[MAX_AMOUNT];

		direction unitDirection[MAX_AMOUNT];

		uint8_t  shieldEnabled[MAX_AMOUNT];
		uint16_t shieldAmount[MAX_AMOUNT];

		uint32_t hitPoints[MAX_AMOUNT];
		uint32_t realHitPoints(int index) { return hitPoints[index] >> 8; }

		uint8_t elevationLevel[MAX_AMOUNT];
		uint8_t unknown1[MAX_AMOUNT];

		uint8_t rank[MAX_AMOUNT];

		uint8_t compAiIdle[MAX_AMOUNT];
		uint8_t humanAiIdle[MAX_AMOUNT];
		uint8_t returnToIdle[MAX_AMOUNT];
		uint8_t attackUnit[MAX_AMOUNT];
		uint8_t attackMove[MAX_AMOUNT];

		uint8_t groundWeapon[MAX_AMOUNT];
		uint8_t maxGroundHits[MAX_AMOUNT];
		uint8_t airWeapon[MAX_AMOUNT];
		uint8_t maxAirHits[MAX_AMOUNT];

		uint8_t aIInternal[MAX_AMOUNT];

		uint32_t specialAbilityFlags[MAX_AMOUNT];
		uint8_t targetAcquisitionRange[MAX_AMOUNT];
		uint8_t sightRange[MAX_AMOUNT];
		uint8_t armorUpgrade[MAX_AMOUNT];
		uint8_t unitSize[MAX_AMOUNT];
		uint8_t armor[MAX_AMOUNT];
		
		uint8_t rightClickAction[MAX_AMOUNT];

		uint16_t readySound[MAX_READY_SOUND_AMOUNT]; // ID 0-105 only
		uint16_t whatSoundStart[MAX_AMOUNT];
		uint16_t whatSoundEnd[MAX_AMOUNT];
		uint16_t pissSoundStart[MAX_PISSED_SOUND_AMOUNT]; // ID 0-105 only
		uint16_t pissSoundEnd[MAX_PISSED_SOUND_AMOUNT]; // ID 0-105 only
		uint16_t yesSoundStart[MAX_YES_SOUND_AMOUNT]; // ID 0-105 only
		uint16_t yesSoundEnd[MAX_YES_SOUND_AMOUNT]; // ID 0-105 only

		uint16_t starEditPlacementBoxDimensions[MAX_AMOUNT][2];

		uint16_t addonHorizontal[MAX_ADDON_AMOUNT]; // ID 106-201 only
		uint16_t addonVertical[MAX_ADDON_AMOUNT]; // ID 106-201 only

		unitDimensions unitDimensions[MAX_AMOUNT]; // left-up-right-down

		uint16_t portrait[MAX_AMOUNT];

		uint16_t mineralCost[MAX_AMOUNT];
		uint16_t vespeneCost[MAX_AMOUNT];
		uint16_t buildTime[MAX_AMOUNT];

		uint16_t unknown2[MAX_AMOUNT];

		uint8_t starEditGroupFlags[MAX_AMOUNT];
		uint8_t supplyProvided[MAX_AMOUNT];
		uint8_t supplyRequired[MAX_AMOUNT];

		uint8_t spaceRequired[MAX_AMOUNT];
		uint8_t spaceProvided[MAX_AMOUNT];

		uint16_t buildScore[MAX_AMOUNT];
		uint16_t destroyScore[MAX_AMOUNT];

		uint16_t unitMapString[MAX_AMOUNT];

		uint8_t broodwarUnitFlag[MAX_AMOUNT];
		
		uint16_t starEditAvailabilityFlags[MAX_AMOUNT];
	};
	#pragma pack (pop)

	UnitTable ReadUnitTable(filesystem::Storage& storage)
	{
		filesystem::StorageFile file;
		UnitTable unitTable;
		
		storage.Read("arr/units.dat", unitTable);

		return unitTable;
	}
}