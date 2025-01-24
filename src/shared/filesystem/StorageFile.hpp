#pragma once

#include <cstdint>
#include <string>

namespace filesystem
{
	enum class FileSeekDir : int
	{
		Beg, End, Cur
	};

	class StorageFile
	{
	public:

		StorageFile();
		StorageFile(StorageFile&& file);

		~StorageFile();

		void Open(void* storageHandle, const char* filePath);

		int ReadBinary(void* data, int size);

		template<typename T>
		void Read(T& data)
		{
			ReadBinary(&data, sizeof(T));
		}

		template<typename T>
		void Read(T* data, int count)
		{
			ReadBinary(data, sizeof(T) * count);
		}

		uint64_t Seek(int64_t offset, FileSeekDir);

		void Close();

		const int GetFileSize();

	private:

		void*       _handle = nullptr;
		std::string _filePath;
		int				  _fileSize = -1;
	};
}