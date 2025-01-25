#include "Assets.hpp"
#include "filesystem/StorageFile.hpp"

#include <SDL_rwops.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>

namespace data
{
	using std::ifstream;
	using namespace filesystem;

	
	Assets::Assets() {}

	Assets::Assets(filesystem::Storage* storage)
		: _storage(storage)
		{}

	int Assets::ReadBytes(const char* path, uint8_t* output) const
	{
		ifstream input(path, std::ios::binary | std::ios::in);

		int size = GetSize(path);

		if (size == -1)
		{
			throw std::runtime_error("Failed to read file!");
		}
		
		int bytesRead = input.readsome(reinterpret_cast<char*>(output), size);

		input.close();

		return bytesRead;
	}

	int Assets::GetSize(const char* path) const
	{
		// TODO: check if file exists

		ifstream input(path, std::ios::binary);

		input.seekg(0, std::ios::end);

		int size = input.tellg();

		if (size == -1)
		{
			throw std::runtime_error("Failed to read file!");
		}

		input.close();

		return size;
	}

	AssetHandle Assets::Open(const char* path)
	{
		StorageFile file;

		_storage->Open(path, file);

		if (!file.IsOpened())
		{
			return nullptr;
		}

		return new StorageFile(std::move(file));
	}

	int Assets::ReadBytes(AssetHandle asset, uint8_t* output, int size) const
	{
		// This can cause memory exceptions, needs better handling
		StorageFile* file = reinterpret_cast<StorageFile*>(asset);

		return file->ReadBinary(output, size);
	}
		
	void Assets::Seek(AssetHandle asset, int offset, filesystem::FileSeekDir dir)
	{
		// This can cause memory exceptions, needs better handling
		StorageFile* file = reinterpret_cast<StorageFile*>(asset);

		file->Seek(offset, dir);
	}

	int Assets::GetSize(AssetHandle asset) const
	{
		// This can cause memory exceptions, needs better handling
		StorageFile* file = reinterpret_cast<StorageFile*>(asset);

		return file->GetFileSize();
	}
	
	int Assets::GetPosition(AssetHandle asset) const
	{
		// This can cause memory exceptions, needs better handling
		StorageFile* file = reinterpret_cast<StorageFile*>(asset);

		return file->GetPosition();
	}

	void Assets::Close(AssetHandle asset)
	{
		// This can cause memory exceptions, needs better handling
		StorageFile* file = reinterpret_cast<StorageFile*>(asset);
		
		file->Close();

		delete file;
	}

	void Assets::AssetToSdlReadIO(SDL_RWops* ops, AssetHandle asset)
	{
		ops->hidden.unknown.data1 = asset;

		ops->size = [](SDL_RWops* ctx) {
			StorageFile* file = reinterpret_cast<StorageFile*>(ctx->hidden.unknown.data1);

			int64_t size = file->GetFileSize();

			return size;
		};

		ops->seek = [](SDL_RWops* ctx, Sint64 offset, int whence) {

			StorageFile* file = reinterpret_cast<StorageFile*>(ctx->hidden.unknown.data1);

			FileSeekDir method;

			switch(whence)
			{
				case RW_SEEK_SET: method = FileSeekDir::Beg; break;
				case RW_SEEK_CUR: method = FileSeekDir::Cur; break;
				case RW_SEEK_END: method = FileSeekDir::End; break;
			}

			int64_t newpos = file->Seek(offset, method);

			return newpos;
		};

		ops->read = [](SDL_RWops* ctx, void* ptr, size_t size, size_t maxnum) {

			StorageFile* file = reinterpret_cast<StorageFile*>(ctx->hidden.unknown.data1);
			size_t read = file->ReadBinary(ptr, size * maxnum);

			return read / size;
		};

		ops->close = [](SDL_RWops* ctx) {

			StorageFile* file = reinterpret_cast<StorageFile*>(ctx->hidden.unknown.data1);
			file->Close();
			return 0;
		};
	}
}