#pragma once

#include <string>

namespace filesystem
{
	class MpqFile
	{
	public:

		~MpqFile();

		void Open(void* archiveHandle, const char* path);

		void ReadBinary(void* data, int size);

		template<typename T>
		void Read(T& data)
		{
			ReadBinary(&data, sizeof(T));
		}

		void Close();

		void Skip(int count);

		bool IsEOF();

		const int GetFileSize();

	private:

		void* _handle;
		std::string _filePath;
		int _fileSize = -1;
		int _offset = 0;
	};
}