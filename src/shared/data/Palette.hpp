#pragma once

#include <data/Common.hpp>

namespace data
{
	const int PALETTE_SIZE = 256;

	struct WpeData
	{
		Color colors[PALETTE_SIZE];
	};

	class Palette
	{
	public:

		Palette();
		Palette(WpeData& data);

		const Color* GetColors() const;
	
		// shifts some palette's colors so that's creates
		//  an illusion of animation
		template<int StartIndex, int Length>
		void cyclePaletteColor()
		{
			Color newPaletteColor[Length];
			memcpy(newPaletteColor + 1, _colors + StartIndex, (Length - 1) * sizeof(Color));

			newPaletteColor[0] = _colors[StartIndex + Length - 1];

			memcpy(_colors + StartIndex, newPaletteColor, Length * sizeof(Color));
		}

	private:

		Color _colors[PALETTE_SIZE];
	};
}