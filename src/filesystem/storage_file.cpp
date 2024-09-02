#include <CascLib.h>
#include <boost/format.hpp>
#include <boost/format/format_fwd.hpp>
#include <stdexcept>

#include "storage_file.hpp"

using std::runtime_error;
using std::string;
using boost::format;

namespace filesystem
{
	
	static void throwErrorMessage(string msg)
	{
		auto error = format("%1%, error code %2%") % msg % GetLastError();

		throw runtime_error(error.str());
	}

	StorageFile::~StorageFile()
	{
		Close();
	}

	void StorageFile::Close()
	{
		if (_handle == nullptr)
			return;

		if (!CascCloseFile(_handle))
		{
			auto message = format("Couldn't close the storage's file %1%") % _filePath;
			throwErrorMessage(message.str());
		}

		_handle = nullptr;
	}

	void StorageFile::Read(void* data, int size)
	{
		DWORD bytesRead;

		if (!CascReadFile(_handle, data, size, &bytesRead))
		{
			auto message = format("Couldn't read file %1%") % _filePath;
			throwErrorMessage(message.str());
		}

		if (bytesRead != size)
		{
			auto message = format("Couldn't fully read all bytes of file %1%") % _filePath;
			throwErrorMessage(message.str());
		}
	}

	void StorageFile::Open(HANDLE storageHandle, const char* filePath)
	{
		if (_handle == nullptr)
		{
			if (!CascOpenFile(storageHandle, filePath, 0, 0, &_handle))
			{
				auto message = format("Couldn't read file %1%") % filePath;
				throwErrorMessage(message.str());
			}
		}
		else
		{
			throw runtime_error("File is already open");
		}
	}

	const int StorageFile::GetFileSize()
	{
		if (_fileSize)
		{
			return _fileSize;
		}
		else // if file's size is unknown then get and record it
		{
			return _fileSize = CascGetFileSize(_handle, nullptr);
		}
	}
}