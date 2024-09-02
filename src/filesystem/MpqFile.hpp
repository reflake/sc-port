#pragma once

#include <string>
#include <winnt.h>

namespace filesystem
{
	class MpqFile
	{
	public:

		~MpqFile();

		void Open(HANDLE archiveHandle, const char* path);

		void Read(void* data, int size);

		template<typename T>
		void Read(T& data)
		{
			Read(&data, sizeof(T));
		}

		void Close();

		const int GetFileSize();

	private:

		HANDLE _handle;
		std::string _filePath;
		int _fileSize = -1;
	};
}