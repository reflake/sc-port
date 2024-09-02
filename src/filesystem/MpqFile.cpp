#include <boost/format.hpp>
#include <boost/format/format_fwd.hpp>
#include <errhandlingapi.h>
#include <stdexcept>

#include "MpqFile.hpp"
#include "StormLib.h"

using boost::format;
using std::string;

namespace filesystem
{
	static void throwErrorMessage(string msg)
	{
		auto error = format("%1%, error code %2%") % msg % GetLastError();
		throw std::runtime_error(error.str());
	}

	MpqFile::~MpqFile()
	{
		Close();
	}

	void MpqFile::Open(HANDLE archiveHandle, const char* path)
	{
		if (!SFileOpenFileEx(archiveHandle, path, 0, &_handle))
		{
			auto message = format("Failed to open archive file %1%") % _filePath;
			throwErrorMessage(message.str());
		}
	}

	void MpqFile::Read(void* data, int size)
	{
		// TODO: need better implementation of the buffer
		char buffer[1024];
		char* dataPtr = reinterpret_cast<char*>(data);
		DWORD bytesRead;

		while(size > 0)
		{
			int count = std::min<int>(size, sizeof(buffer));
			
			if (!SFileReadFile(_handle, buffer, count, &bytesRead, NULL))
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

	void MpqFile::Close()
	{
		if (_handle == nullptr)
			return;

		if (!SFileCloseFile(_handle))
		{
			throwErrorMessage("Couldn't close the archive file");
		}

		_handle = nullptr;
	}
}