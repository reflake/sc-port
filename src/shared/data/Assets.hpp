#pragma once

#include "filesystem/Storage.hpp"
#include <SDL_rwops.h>
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
		void Seek(AssetHandle, int offset, filesystem::FileSeekDir dir);
		int GetSize(AssetHandle) const;
		int GetPosition(AssetHandle) const;
		void Close(AssetHandle);

		void AssetToSdlReadIO(SDL_RWops*, AssetHandle);

	private:

		filesystem::Storage* _storage;
	};
}