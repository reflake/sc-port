#include "Assets.hpp"

#include <filesystem>
#include <fstream>
#include <stdexcept>

namespace data
{
	using std::ifstream;

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
}