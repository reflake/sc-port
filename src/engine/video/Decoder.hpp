#pragma once

#include "Video.hpp"
#include "data/Assets.hpp"

namespace video
{
	enum class CodecType
	{
		VP9
	};

	class A_HwDecoderInterface;

	class Decoder
	{
	public:

		Decoder(CodecType);
		~Decoder();

		void Initialize();
		void DecodeFrame(int size, uint8_t* input, uint8_t* output);

	private:

		A_HwDecoderInterface* _codec = nullptr;

		CodecType     _codecType;
	};

	class A_DecoderInterfaceFactory
	{
	public:

		virtual A_HwDecoderInterface* Create() = 0;
	};

	extern void RegisterCodecFactory(CodecType, A_DecoderInterfaceFactory*);

	template<class T>
	class DecoderInterfaceFactory : public A_DecoderInterfaceFactory
	{
	public:

		DecoderInterfaceFactory(CodecType codecType)
		{
			RegisterCodecFactory(codecType, this);
		}

		A_HwDecoderInterface* Create() override
		{
			return new T();
		}
	};

	class A_HwDecoderInterface
	{
	public:
	
		virtual ~A_HwDecoderInterface() {};

		virtual void DecodeFrame(int size, uint8_t* input, uint8_t* output) = 0;
		virtual void Release() = 0;
	};
}