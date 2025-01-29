#include "TextStrings.hpp"

#include <memory>

#include "Common.hpp"

using filesystem::Storage;
using filesystem::StorageFile;

using std::vector;

namespace data
{
	void StringsTable::Load(std::shared_ptr<uint8_t[]> data, uint32_t size)
	{
		StreamReader reader(data, size);

		rawData = data;

		uint16_t amount;
		reader.Read(amount);
		
		entries = vector<const char*>(amount);

		for(int i = 0; i < amount; i++)
		{
			uint16_t fileOffset;
			reader.Read(fileOffset);

			entries[i] = reinterpret_cast<char*>(&rawData[fileOffset]);
		}
	}

	void ReadTextStringsTable(Storage& storage, const char* path, StringsTable& out)
	{
		StorageFile file;
		storage.Open(path, file);

		int fileSize = file.GetFileSize();
		auto rawData = std::make_shared<uint8_t[]>(fileSize);

		file.ReadBinary(rawData.get(), fileSize);

		out.Load(rawData,fileSize);
	}
}