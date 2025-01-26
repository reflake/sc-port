#include "Decoder.hpp"
#include <unordered_map>

namespace video
{
	static std::unordered_map<CodecType, A_DecoderInterfaceFactory*> codecFactories;

	void RegisterCodecFactory(CodecType type, A_DecoderInterfaceFactory* factory)
	{
		codecFactories[type] = factory;
	}

	Decoder::Decoder(CodecType type):
		_codecType(type)
	{}

	void Decoder::Initialize()
	{
		auto factory = codecFactories[_codecType];

		_codec = factory->Create();
	}

	void Decoder::DecodeFrame(int size, uint8_t* input, uint8_t* output)
	{
		_codec->DecodeFrame(size, input, output);
	}

	Decoder::~Decoder()
	{
		_codec->Release();
	}
}