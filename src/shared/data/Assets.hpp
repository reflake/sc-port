#pragma once

#include "filesystem/Storage.hpp"
#include <cstdint>

namespace data
{
	typedef void* AssetHandle;

	class Assets
	{
	public:

		Assets();
		Assets(filesystem::Storage*);

		int ReadBytes(const char* path, uint8_t* output) const;
		int GetSize(const char* path) const;
		
		AssetHandle Open(const char* path);
		int ReadBytes(AssetHandle, uint8_t* output, int size) const;
		int GetSize(AssetHandle) const;
		void Close(AssetHandle);

	private:

		filesystem::Storage* _storage;
	};
}