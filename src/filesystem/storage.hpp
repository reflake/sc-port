#pragma once

#include <memory>
#include <winnt.h>

#include "StorageFile.hpp"

namespace filesystem
{
	class Storage
	{
	public:

		Storage(const char* path);
		~Storage();

		void Read(const char* path, void* data, int size);
		
		template<typename T>
		void Read(const char* path, T& data)
		{
			Read(path, &data, sizeof(T));
		}

		void Open(const char* path, StorageFile& file);

		void Close();

	private:

		HANDLE _storage;
	};
}