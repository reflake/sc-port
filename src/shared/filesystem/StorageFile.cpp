#include <algorithm>
#include <CascLib.h>
#include <boost/format.hpp>
#include <boost/format/format_fwd.hpp>
#include <stdexcept>
#include <windows.h>
#include <winnt.h>

#include "StorageFile.hpp"

using std::runtime_error;
using std::string;
using boost::format;

namespace filesystem
{
	StorageFile::StorageFile() {}
	
	StorageFile::StorageFile(StorageFile&& file)
		: _handle(file._handle), _fileSize(file._fileSize)
	{
		file._handle = nullptr;
	}
	
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

	int StorageFile::ReadBinary(void* data, int size)
	{
		char* dataPtr = reinterpret_cast<char*>(data);
		DWORD bytesRead;

		if (!CascReadFile(_handle, data, size, &bytesRead))
		{
			auto message = format("Couldn't read file %1%") % _filePath;
			throwErrorMessage(message.str());
		}

		return bytesRead;
	}

	uint64_t StorageFile::Seek(int64_t offset, FileSeekDir dir)
	{
		uint64_t output;
		DWORD    method;

		switch(dir)
		{
			case FileSeekDir::Beg: method = FILE_BEGIN; break;
			case FileSeekDir::Cur: method = FILE_CURRENT; break;
			case FileSeekDir::End: method = FILE_END; break;
		}

		if (!CascSetFilePointer64(_handle, offset, &output, method))
		{
			auto message = format("Couldn't seek file %1%") % _filePath;
			throwErrorMessage(message.str());
		}

		return output;
	}

	uint64_t StorageFile::GetPosition()
	{
		// dirty hack!
		return Seek(0, FileSeekDir::Cur);
	}

	void StorageFile::Open(void* storageHandle, const char* filePath)
	{
		if (_handle == nullptr)
		{
			if (!CascOpenFile(storageHandle, filePath, 0, 0, &_handle))
			{
				_handle = nullptr;
			}
		}
		else
		{
			throw runtime_error("File is already open");
		}
	}

	const int StorageFile::GetFileSize()
	{
		if (_handle == nullptr)
		{
			throw runtime_error("File is not opened");
		}

		if (_fileSize != -1)
		{
			return _fileSize;
		}
		else // if file's size is unknown then get and record it
		{
			return _fileSize = CascGetFileSize(_handle, nullptr);
		}
	}

	bool StorageFile::IsEOF()
	{
		return GetPosition() >= GetFileSize();
	}

	bool StorageFile::IsOpened() const
	{
		return _handle != nullptr;
	}
}