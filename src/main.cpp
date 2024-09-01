#include <CascLib.h>
#include <fileapi.h>
#include <handleapi.h>
#include <iostream>
#include <minwindef.h>
#include <ostream>
#include <stdexcept>
#include <winnt.h>

#include "meta/Unit.h"

using std::cout;
using std::runtime_error;

int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		cout << "An argument is required: <storage path>" << std::endl;
		return -1;
	}

	auto storagePath = argv[1];
	auto outputPath = argv[2];

	HANDLE storage;

	// Check CascLib version if it's actual
	static_assert(CASCLIB_VERSION >= 0x210, "CascLib version should be 2.1 or above");

	if (!CascOpenStorage(storagePath, 0, &storage))
	{
		throw runtime_error("Couldn't open game storage");
	}

	HANDLE storageFile;

	if (!CascOpenFile(storage, "arr/units.dat", 0, 0, &storageFile))
	{
		throw runtime_error("Couldn't unit meta data file");
	}

	const int MARINE_ID = 0x00;

	meta::UnitTable unitTable;
	DWORD bytesRead;
	DWORD metaDataFileSize;

	metaDataFileSize = CascGetFileSize(storageFile, NULL);

	CascReadFile(storageFile, &unitTable, metaDataFileSize, &bytesRead);

	if (bytesRead == metaDataFileSize)
	{
		cout << "Found marine data, he's HP equals " << unitTable.realHitPoints(MARINE_ID) << std::endl;
	}
	else 
	{
		throw runtime_error("Couldn't read marine data");
	}

	CascCloseFile(storageFile);
	CascCloseStorage(storage);
}