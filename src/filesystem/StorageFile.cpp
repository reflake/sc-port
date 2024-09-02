#include <CascLib.h>
#include <algorithm>
#include <boost/format.hpp>
#include <boost/format/format_fwd.hpp>
#include <stdexcept>

#include "StorageFile.hpp"

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
		// TODO: need better implementation of the buffer
		char buffer[1024];
		char* dataPtr = reinterpret_cast<char*>(data);
		DWORD bytesRead;

		while(size > 0)
		{
			int count = std::min<int>(size, sizeof(buffer));

			if (!CascReadFile(_handle, buffer, count, &bytesRead))
			{
				auto message = format("Couldn't read file %1%") % _filePath;
				throwErrorMessage(message.str());
			}

			if (bytesRead != count)
			{
				auto message = format("Couldn't fully read all bytes of file %1%") % _filePath;
				throwErrorMessage(message.str());
			}

			memcpy(dataPtr, buffer, count);

			size -= sizeof(buffer);
			dataPtr += sizeof(buffer);
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
		if (_fileSize != -1)
		{
			return _fileSize;
		}
		else // if file's size is unknown then get and record it
		{
			return _fileSize = CascGetFileSize(_handle, nullptr);
		}
	}
}