#pragma once

#include "data/Assets.hpp"
#include "data/Common.hpp"

namespace meta
{
	struct PortraitTable
	{
		data::Array<uint32_t, 0, 109> fidgetStringPtr;
		data::Array<uint32_t, 0, 109> talkingStringPtr;
		data::Array<uint8_t, 0, 109>  fidgetChange;
		data::Array<uint8_t, 0, 109>  talkingChange;
		data::Array<uint8_t, 0, 109>  unknown1;
		data::Array<uint8_t, 0, 109>  unknown2;
	};

	extern void ReadPortraitTable(data::Assets*, PortraitTable& out);
}