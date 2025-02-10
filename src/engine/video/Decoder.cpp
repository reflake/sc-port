#include "Decoder.hpp"

namespace video
{
	Decoder::Decoder(CodecType type):
		_codecType(type)
	{}

	void Decoder::Initialize()
	{
		auto factory = DecoderFactoryDictionary::Instance.FindFactory(_codecType);

		_codec = factory->Create();
	}

	void Decoder::DecodeFrame(int size, uint8_t* input, uint8_t* output)
	{
		_codec->DecodeFrame(size, input, output);
	}

	Decoder::~Decoder()
	{
		_codec->Release();

		delete _codec;
	}
}