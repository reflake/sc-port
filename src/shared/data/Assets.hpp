#pragma once

#include <cstdint>

namespace data
{
	class Assets
	{
	public:

		int ReadBytes(const char* path, uint8_t* output) const;
		int GetSize(const char* path) const;
	};
}