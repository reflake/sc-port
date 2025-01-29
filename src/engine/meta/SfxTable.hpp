#pragma once

#include "data/Common.hpp"
#include <cstdint>

namespace meta
{
	class SfxTable
	{
		data::Array<uint32_t, 0, 1143> sfx;
		data::Array<uint8_t, 0, 1143>  unknown1;
		data::Array<uint16_t, 0, 1143> unknown2;
		data::Array<uint8_t, 0, 1143>  unknown3;
		data::Array<uint8_t, 0, 1143>  unknown4;
	};
}