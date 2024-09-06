#include "Sprite.hpp"

namespace data
{
	void ReadSpriteTable(filesystem::Storage& storage, SpriteTable& spriteTable)
	{
		filesystem::StorageFile file;
		storage.Open("arr/sprites.dat", file);

		file.Read(spriteTable.imageID);
		file.Read(spriteTable.healthBarLength);
		file.Read(spriteTable.unknown);
		file.Read(spriteTable.visible);
		file.Read(spriteTable.selectionCircleImage);
		file.Read(spriteTable.selectionCircleVerticalOffset);
	}
};