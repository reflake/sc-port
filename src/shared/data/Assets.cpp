#include "Assets.hpp"
#include "filesystem/StorageFile.hpp"

#include <filesystem>
#include <fstream>
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

		return new StorageFile(std::move(file));
	}

	int Assets::ReadBytes(AssetHandle asset, uint8_t* output, int size) const
	{
		// This can cause memory exceptions, needs better handling
		StorageFile* file = reinterpret_cast<StorageFile*>(asset);

		return file->ReadBinary(output, size);
	}

	

	int Assets::GetSize(AssetHandle asset) const
	{
		// This can cause memory exceptions, needs better handling
		StorageFile* file = reinterpret_cast<StorageFile*>(asset);

		return file->GetFileSize();
	}

	void Assets::Close(AssetHandle asset)
	{
		// This can cause memory exceptions, needs better handling
		StorageFile* file = reinterpret_cast<StorageFile*>(asset);
		
		file->Close();

		delete file;
	}
}