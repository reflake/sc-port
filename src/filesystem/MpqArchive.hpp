#pragma once

#include <winnt.h>

#include "MpqFile.hpp"

namespace filesystem
{
	class MpqArchive
	{
	public:

		MpqArchive(const char* filePath);
		~MpqArchive();

		void Read(const char* path, void* data, int size);

		template<typename T>
		void Read(const char* path, T& data)
		{
			Read(path, &data, sizeof(T));
		}

		void Open(const char* path, MpqFile& file);

		void Close();

	private:

		HANDLE _archive;
	};
}