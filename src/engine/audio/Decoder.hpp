#pragma once

#include "utility/Factory.hpp"
#include <cstdint>

namespace audio
{
	enum class CodecType
	{
		Vorbis
	};

	class A_HwDecoder;

	class Decoder
	{
	public:

		Decoder(CodecType);
		~Decoder();

		void Initialize();
		void Decode(int size, uint8_t* input, uint8_t output);

	private:

		A_HwDecoder* _codec = nullptr;

		CodecType _type;
	};

	// Looks like shit
	class DecoderFactoryDictionary : public FactoryDictionary<A_HwDecoder, CodecType>
	{};

	class A_HwDecoder
	{
	public:

		virtual ~A_HwDecoder() {};

		virtual void Decode(int size, uint8_t* input, uint8_t output);
	};
}