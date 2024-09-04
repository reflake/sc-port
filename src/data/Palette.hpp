#pragma once

#include <glm/vec4.hpp>

namespace data
{
	typedef glm::vec<4, uint8_t> Color;

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

	private:

		Color _colors[PALETTE_SIZE];
	};
}