#pragma once

#include <cstdint>
#include <string>

namespace data
{
	class Assets
	{
	public:

		int ReadBytes(const char* path, uint8_t* output);
		int GetSize(const char* path);
	};
}