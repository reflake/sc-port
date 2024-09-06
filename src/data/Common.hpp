#pragma once

#include <cassert>
#include <memory>

namespace data
{
	typedef uint16_t tileGroupID;
	typedef uint8_t  tileVariation;
	typedef uint16_t tileID;

	enum class Tileset: uint16_t { 
		Badlands, SpacePlatform, Installation, 
		Ashworld, Jungle, Desert, Arctic,
		Twilight
	};

	extern bool HasTileSetWater(Tileset tileset);

	class StreamReader
	{
	public:

		StreamReader(std::shared_ptr<uint8_t[]> data, int dataSize) : _data(data), _dataSize(dataSize) {}

		void ReadBinary(void* out, int size)
		{
			assert(size + _offset <= _dataSize);

			memcpy(out, _data.get() + _offset, size);

			_offset += size;

		}

		template<typename T>
		void Read(T& data)
		{
			ReadBinary(&data, sizeof(T));
		}

		void Skip(int amount)
		{
			_offset += amount;

			assert(_offset <= _dataSize);
		}

		bool IsEOF() { return _offset == _dataSize; }

	private:

		std::shared_ptr<uint8_t[]> _data;
		int _dataSize;
		int _offset = 0;
	};

	template<typename T, int StartIndex, int EndIndex>
	struct Array
	{
	public:

		inline T& Get(int index) const
		{
			return elements[index - StartIndex];
		}

		inline bool HasElement(int index) const
		{
			return StartIndex <= index && index <= EndIndex;
		}

		inline T& operator[](int index) const 
		{
			return Get(index);
		}

	private:

		T elements[EndIndex - StartIndex + 1];
	};
}