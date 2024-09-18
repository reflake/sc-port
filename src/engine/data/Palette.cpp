#include <cstring>

#include "Palette.hpp"

namespace data {

	Palette::Palette()
	{
	}

	Palette::Palette(WpeData& data)
	{
		for(int i = 0; i < PALETTE_SIZE; i++)
		{
			_colors[i] = data.colors[i];
			_colors[i].a = 0xFF;
		}

		// First color of palette is transparent
		_colors[0].a = 0x00;
	}

	const Color* Palette::GetColors() const
	{
		return _colors;
	}
}