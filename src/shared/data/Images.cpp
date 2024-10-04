#include <cassert>

#include "Images.hpp"

namespace data
{
	void ReadImagesTable(filesystem::Storage& storage, ImagesTable& table)
	{
		filesystem::StorageFile file;

		storage.Open("arr/images.dat", file);

		assert(file.GetFileSize() == MAX_IMAGES_AMOUNT * 38);

		file.Read(table.grpID);
		file.Read(table.turns);
		file.Read(table.selectable);
		file.Read(table.useFullIScript);
		file.Read(table.drawIfCloaked);
		file.Read(table.drawFunction);
		file.Read(table.remapping);
		file.Read(table.iScriptID);
		file.Read(table.shieldOverlay);
		file.Read(table.attackOverlay);
		file.Read(table.damageOverlay);
		file.Read(table.specialOverlay);
		file.Read(table.landingOverlay);
		file.Read(table.liftOffOverlay);
	}
}