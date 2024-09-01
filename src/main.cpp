#include <CascLib.h>
#include <fileapi.h>
#include <handleapi.h>
#include <iostream>
#include <minwindef.h>
#include <ostream>
#include <stdexcept>
#include <winnt.h>

using std::cout;
using std::runtime_error;

void constexpr CheckCascLibVersion()
{
	static_assert(CASCLIB_VERSION >= 0x210, "CascLib version should be 2.1 or above");
}

int main(int argc, char* argv[])
{
	if (argc < 3)
	{
		cout << "Two arguments required: <storage path> <output file>" << std::endl;
		return -1;
	}

	auto storagePath = argv[1];
	auto outputPath = argv[2];

	HANDLE storage;

	CheckCascLibVersion();

	if (!CascOpenStorage(storagePath, 0, &storage))
	{
		throw runtime_error("Couldn't open game storage");
	}

	HANDLE storageFile;

	if (!CascOpenFile(storage, "sound/terran/marine/tmapss00.wav", 0, 0, &storageFile))
	{
		throw runtime_error("Couldn't read sound file");
	}

	auto outputFile = CreateFile(outputPath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);

	if (outputFile == INVALID_HANDLE_VALUE)
	{
		throw runtime_error("Couldn't create output file");
	}

	char  buffer[1000];
	DWORD bytesRead;

	do {

		CascReadFile(storageFile, buffer, sizeof(buffer), &bytesRead);

		DWORD bytesWritten;

		if (bytesRead > 0)
		{
			WriteFile(outputFile, buffer, bytesRead, &bytesWritten, NULL);

			if (bytesWritten != bytesRead)
			{
				throw runtime_error("Couldn't write to output file");
			}
		}

	} while (bytesRead != 0);

	CascCloseFile(storageFile);
	CascCloseStorage(storage);
	CloseHandle(outputFile);
}