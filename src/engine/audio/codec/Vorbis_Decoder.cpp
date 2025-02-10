#include "../Decoder.hpp"

namespace audio
{
	class Vorbis_Decoder : public A_HwDecoder
	{
	public:

		
	};

	// Looks like shit, but I don't want to explicitly define
	//  in header files every decoder
	static DecoderFactoryDictionary::Factory<Vorbis_Decoder> factory(CodecType::Vorbis);
}