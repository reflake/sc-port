#pragma once

#include <string>
#include <winnt.h>

namespace filesystem
{
	class StorageFile
	{
	public:

		~StorageFile();

		void Open(HANDLE storageHandle, const char* filePath);

		void Read(void* data, int size);

		template<typename T>
		void Read(T& data)
		{
			Read(&data, sizeof(T));
		}

		void Close();

		const int GetFileSize();

	private:

		HANDLE      _handle = nullptr;
		std::string _filePath;
		int				  _fileSize = -1;
	};
}