
#include <array>
#include <stdexcept>
#include <vpx/vpx_codec.h>
#include <video/Decoder.hpp>
#include <vpx/vp8dx.h>
#include <vpx/vpx_decoder.h>

namespace video
{
	using std::runtime_error;

	class Vp9_Decoder : public A_HwDecoder
	{
	public:

		Vp9_Decoder();
		~Vp9_Decoder();
		
		void DecodeFrame(int size, uint8_t* input, uint8_t* output) override;
		void Release() override;

	private:
		
		vpx_codec_ctx_t _codecContext;
	};

	// This line register factory in global dictionary of factories
	// Looks like shit, but I don't want to explicitly define
	//  in header files every decoder
	static DecoderFactoryDictionary::Factory<Vp9_Decoder> factory(CodecType::VP9);

	Vp9_Decoder::Vp9_Decoder()
	{
		if (vpx_codec_dec_init(&_codecContext, vpx_codec_vp9_dx(), nullptr, 0))
		{
			throw runtime_error("Failed to initialize VP9 decoder");
		}
	}

	Vp9_Decoder::~Vp9_Decoder()
	{}

	int GetPlaneWidth(const vpx_image_t *img, int plane) {
		if (plane > 0 && img->x_chroma_shift > 0)
			return (img->d_w + 1) >> img->x_chroma_shift;
		else
			return img->d_w;
	}

	int GetPlaneHeight(const vpx_image_t *img, int plane) {
		if (plane > 0 && img->y_chroma_shift > 0)
			return (img->d_h + 1) >> img->y_chroma_shift;
		else
			return img->d_h;
	}

	void Vp9_Decoder::DecodeFrame(int size, uint8_t* input, uint8_t* output)
	{
		if (vpx_codec_decode(&_codecContext, input, size, nullptr, 0))
		{
			throw runtime_error("Failed to decode frame");
		}
		
		vpx_codec_iter_t iter = nullptr;
		vpx_image_t      *img = nullptr;

		// memset(pixels.get(), 0, video.width * video.height * pixelSize);

		// Color order (G B R)
		const std::array<int, 3> channelMap = { 1, 2, 0 };
		const int                pixelSize = 4;

		while ((img = vpx_codec_get_frame(&_codecContext, &iter)) != nullptr)
		{
			if (img->cs != VPX_CS_SRGB)
			{
				throw std::runtime_error("Unsupported color space");
			}

			if (img->fmt & VPX_IMG_FMT_HIGHBITDEPTH)
			{
				throw std::runtime_error("Unsupported depth format");
			}

			for(int i = 0; i < 3; i++)
			{
				const uint8_t* buf = img->planes[i];
				const int stride = img->stride[i];

				int w = GetPlaneWidth(img, i);
				int h = GetPlaneHeight(img, i);

				// Assuming that for nv12 we write all chroma data at once
				if (img->fmt == VPX_IMG_FMT_NV12 && i > 1) break;
				// Fixing NV12 chroma width if it is odd
				if (img->fmt == VPX_IMG_FMT_NV12 && i == 1) w = (w + 1) & ~1;

				for (int y = 0; y < h; y++)
				{
					for (int x = 0; x < w; x++)
					{
						int pixelIndex = x + y * w;

						output[pixelIndex * pixelSize + channelMap[i]] = buf[x];
					};

					buf += stride;
				}
			}
		}
	}

	void Vp9_Decoder::Release()
	{
		if (vpx_codec_destroy(&_codecContext))
		{
			throw runtime_error("Failed to destroy VP9 codec context");
		}
	}
}