#include "Sprite.hpp"

namespace data
{
	void ReadSpriteTable(filesystem::Storage& storage, SpriteTable& spriteTable)
	{
		storage.Read("arr/sprites.dat", spriteTable);
	}
};