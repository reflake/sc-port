#pragma once

#include <cstdint>
#include <memory>
#include <vector>

#include <filesystem/Storage.hpp>

namespace data
{
	struct TextStringsTable
	{
		std::vector<const char*> entries;
		std::shared_ptr<uint8_t[]> rawData;

		void Load(std::shared_ptr<uint8_t[]> data, uint32_t size);
	};

	extern void ReadTextStringsTable(filesystem::Storage& storage, const char* path, TextStringsTable& out);
};