
#include <boost/format.hpp>
#include <boost/format/format_fwd.hpp>
#include <errhandlingapi.h>
#include <string>
#include <StormLib.h>

#include "MpqArchive.hpp"

using boost::format;
using std::string;

namespace filesystem
{

	static void throwErrorMessage(const char* msg)
	{
		auto error = format("%1%, error code %2%") % msg % GetLastError();
	}

	MpqArchive::MpqArchive(const char* filePath)
	{
		if (!SFileOpenArchive(filePath, 0, 0, &_archive))
		{
			throwErrorMessage("Failed to open the archive");
		}
	}

	MpqArchive::~MpqArchive()
	{
		Close();
	}

	void MpqArchive::Read(const char* path, void* data, int size)
	{
		MpqFile file;

		Open(path, file);

		file.Read(data, size);
	}

	void MpqArchive::Open(const char* path, MpqFile& file)
	{
		file.Open(_archive, path);
	}

	void MpqArchive::Close()
	{
		if (_archive == nullptr)
			return;
		
		if (!SFileCloseArchive(_archive))
		{
			throwErrorMessage("Couldn't close archive");
		}
		
		_archive = nullptr;
	}

}