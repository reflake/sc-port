#include "Decoder.hpp"
#include <unordered_map>

namespace audio
{
	static std::unordered_map<CodecType, A_HwDecoderFactory*> codecFactories;

	void RegisterCodecFactory(CodecType type, A_HwDecoderFactory* factory)
	{
		codecFactories[type] = factory;
	}

	Decoder::Decoder(CodecType type) :
		_type(type)
	{
	}

	void Decoder::Initialize()
	{
		auto factory = codecFactories[_type];

		_codec = factory->Create();
	}

	void Decoder::Decode(int size, uint8_t* input, uint8_t output)
	{
		_codec->Decode(size, input, output);
	}

	Decoder::~Decoder()
	{
		delete _codec;
	}
}