#pragma once

#include <cassert>
#include <memory>

#include <glm/ext/vector_float2.hpp>
#include <glm/vec4.hpp>

namespace data
{
	typedef glm::vec<4, uint8_t> Color;

	typedef uint16_t  tileGroupID;
	typedef uint8_t   tileVariation;
	typedef uint16_t  tileID;
	typedef glm::vec2 position;

	class StreamReader
	{
	public:

		StreamReader(const std::shared_ptr<uint8_t[]> data, int dataSize) : _data(data), _dataSize(dataSize) {}

		inline void ReadBinary(void* out, int size)
		{
			assert(size + _offset <= _dataSize);

			memcpy(out, _data.get() + _offset, size);

			_offset += size;
		}

		template<typename T>
		inline void Read(T& data)
		{
			ReadBinary(&data, sizeof(T));
		}

		template<typename T>
		inline void Read(T* data, int count)
		{
			ReadBinary(data, sizeof(T) * count);
		}

		inline void Skip(int amount)
		{
			_offset += amount;

			assert(_offset <= _dataSize);
		}

		inline void SetPointer(int index)
		{
			_offset = index;
		}

		inline const int GetPointer() const
		{
			return _offset;
		}

		bool IsEOF() { return _offset == _dataSize; }

	private:

		const std::shared_ptr<uint8_t[]> _data;
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

		constexpr int Length() const
		{
			return EndIndex - StartIndex + 1;
		}

	private:

		T elements[EndIndex - StartIndex + 1];
	};
}