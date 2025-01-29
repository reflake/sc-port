#pragma once

#include <cstdint>
#include <glm/vec4.hpp>

#include <data/Common.hpp>
#include <filesystem/Storage.hpp>

namespace meta
{
	typedef uint8_t direction;

	const int MAX_UNIT_AMOUNT = 228;
	const int MAX_UNIT_ADDON_AMOUNT = 96;

	typedef glm::vec<4, uint16_t> unitDimensions;

	struct UnitTable
	{
		uint8_t flingyID[MAX_UNIT_AMOUNT];

		uint16_t subunitIDs[2][MAX_UNIT_AMOUNT];

		data::Array<uint16_t, 106, 201> infestationID;

		uint32_t constructionAnimation[MAX_UNIT_AMOUNT];

		direction unitDirection[MAX_UNIT_AMOUNT];

		uint8_t  shieldEnabled[MAX_UNIT_AMOUNT];
		uint16_t shieldAmount[MAX_UNIT_AMOUNT];

		uint32_t hitPoints[MAX_UNIT_AMOUNT];
		uint32_t realHitPoints(int index) { return hitPoints[index] >> 8; }

		uint8_t elevationLevel[MAX_UNIT_AMOUNT];
		uint8_t unknown1[MAX_UNIT_AMOUNT];

		uint8_t rank[MAX_UNIT_AMOUNT];

		uint8_t compAiIdle[MAX_UNIT_AMOUNT];
		uint8_t humanAiIdle[MAX_UNIT_AMOUNT];
		uint8_t returnToIdle[MAX_UNIT_AMOUNT];
		uint8_t attackUnit[MAX_UNIT_AMOUNT];
		uint8_t attackMove[MAX_UNIT_AMOUNT];

		uint8_t groundWeapon[MAX_UNIT_AMOUNT];
		uint8_t maxGroundHits[MAX_UNIT_AMOUNT];
		uint8_t airWeapon[MAX_UNIT_AMOUNT];
		uint8_t maxAirHits[MAX_UNIT_AMOUNT];

		uint8_t aIInternal[MAX_UNIT_AMOUNT];

		uint32_t specialAbilityFlags[MAX_UNIT_AMOUNT];
		uint8_t targetAcquisitionRange[MAX_UNIT_AMOUNT];
		uint8_t sightRange[MAX_UNIT_AMOUNT];
		uint8_t armorUpgrade[MAX_UNIT_AMOUNT];
		uint8_t unitSize[MAX_UNIT_AMOUNT];
		uint8_t armor[MAX_UNIT_AMOUNT];
		
		uint8_t rightClickAction[MAX_UNIT_AMOUNT];

		data::Array<uint16_t, 0, 105> readySound;
		uint16_t whatSoundStart[MAX_UNIT_AMOUNT];
		uint16_t whatSoundEnd[MAX_UNIT_AMOUNT];
		data::Array<uint16_t, 0, 105> pissSoundStart;
		data::Array<uint16_t, 0, 105> pissSoundEnd;
		data::Array<uint16_t, 0, 105> yesSoundStart;
		data::Array<uint16_t, 0, 105> yesSoundEnd;

		uint16_t starEditPlacementBoxDimensions[MAX_UNIT_AMOUNT][2];

		data::Array<uint16_t, 106, 201> addonHorizontal;
		data::Array<uint16_t, 106, 201> addonVertical;

		unitDimensions unitDimensions[MAX_UNIT_AMOUNT]; // left-up-right-down

		data::Array<uint16_t, 0, MAX_UNIT_AMOUNT - 1> portrait;

		uint16_t mineralCost[MAX_UNIT_AMOUNT];
		uint16_t vespeneCost[MAX_UNIT_AMOUNT];
		uint16_t buildTime[MAX_UNIT_AMOUNT];

		uint16_t unknown2[MAX_UNIT_AMOUNT];

		uint8_t starEditGroupFlags[MAX_UNIT_AMOUNT];
		uint8_t supplyProvided[MAX_UNIT_AMOUNT];
		uint8_t supplyRequired[MAX_UNIT_AMOUNT];

		uint8_t spaceRequired[MAX_UNIT_AMOUNT];
		uint8_t spaceProvided[MAX_UNIT_AMOUNT];

		uint16_t buildScore[MAX_UNIT_AMOUNT];
		uint16_t destroyScore[MAX_UNIT_AMOUNT];

		uint16_t unitMapString[MAX_UNIT_AMOUNT];

		uint8_t broodwarUnitFlag[MAX_UNIT_AMOUNT];
		
		uint16_t starEditAvailabilityFlags[MAX_UNIT_AMOUNT];
	};

	static_assert(sizeof(UnitTable) == 19876, "UnitTable size should be equal to 19876");

	extern void ReadUnitTable(filesystem::Storage& storage, UnitTable& table);
}