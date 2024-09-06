#include "TextStrings.hpp"

#include <memory>

#include "Common.hpp"

using filesystem::Storage;
using filesystem::StorageFile;

using std::vector;

namespace data
{

	void ReadTextStringsTable(Storage& storage, const char* path, TextStringsTable& out)
	{
		StorageFile file;
		storage.Open(path, file);

		out.rawData = std::make_shared<uint8_t[]>(file.GetFileSize());
		file.ReadBinary(out.rawData.get(), file.GetFileSize());

		StreamReader reader(out.rawData, file.GetFileSize());

		uint16_t amount;
		reader.Read(amount);

		out.entries = vector<const char*>(amount);

		for(int i = 0; i < amount; i++)
		{
			uint16_t fileOffset;
			reader.Read(fileOffset);

			out.entries[i] = reinterpret_cast<char*>(&out.rawData[fileOffset]);
		}
	}
}