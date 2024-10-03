#include <CascLib.h>
#include <boost/format.hpp>
#include <boost/format/format_fwd.hpp>
#include <errhandlingapi.h>
#include <minwindef.h>
#include <stdexcept>
#include <string>
#include <winnt.h>

#include "Storage.hpp"
#include "StorageFile.hpp"
#include "StormLib.h"

using std::string;
using std::runtime_error;

// Check CascLib version if it's actual
static_assert(CASCLIB_VERSION >= 0x210, "CascLib version should be 2.1 or above");

namespace filesystem
{

	static void throwErrorMessage(string msg)
	{
		auto error = boost::format("%1%, error code %2%") % msg % GetLastError();

		throw runtime_error(error.str());
	}

	Storage::Storage(const char* path)
	{
		if (!CascOpenStorage(path, 0, &_storage))
		{
			throwErrorMessage("Failed to open the game storage");
		}
	}

	Storage::~Storage()
	{
		Close();
	}

	void Storage::Read(const char* path, void* data, int size)
	{
		StorageFile file;

		Open(path, file);

		file.ReadBinary(data, size);
	}

	void Storage::Open(const char* path, StorageFile& file)
	{
		file.Open(_storage, path);
	}

	void Storage::Open(const boost::format& path, StorageFile& file)
	{
		file.Open(_storage, path.str().c_str());
	}

	void Storage::Close()
	{
		if (_storage == nullptr)
			return;
		
		if (!CascCloseStorage(_storage))
		{
			throwErrorMessage("Couldn't close storage");
		}

		_storage = nullptr;
	}
}