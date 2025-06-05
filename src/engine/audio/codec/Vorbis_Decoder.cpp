#include "../Decoder.hpp"
#include "data/Assets.hpp"
#include "filesystem/StorageFile.hpp"

#include <cstdio>
#include <stdexcept>
#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>

namespace audio
{
	struct AssetData
	{
		data::AssetHandle _assetHandle;
		data::Assets*     _assets;
	};

	class Vorbis_Decoder : public A_HwDecoder
	{
	public:

		Vorbis_Decoder(data::AssetHandle, data::Assets*);
		~Vorbis_Decoder() override;

		void Decode(int size, uint8_t* output) override;
		
	private:

		AssetData       _assetData;
		OggVorbis_File  _vf;
		int _channels, _rate;
		int _totalSampleCount;
	};

	static size_t Read(void* output, size_t size, size_t amount, void* asset);
	static int    Seek(void* asset, ogg_uint64_t offset, int whence);
	static long   Tell(void* asset);

	static ov_callbacks readCallbacks = {
		(size_t (*)(void *, size_t, size_t, void *))  Read,
		(int (*)(void *, ogg_int64_t, int))           Seek,
		(int (*)(void *))                             nullptr,
		(long (*)(void *))                            Tell
	};

	// Looks like shit, but I don't want to explicitly define
	//  in header files every decoder
	static DecoderFactoryDictionary::Factory<Vorbis_Decoder> factory(CodecType::Vorbis);

	Vorbis_Decoder::Vorbis_Decoder(data::AssetHandle assetHandle, data::Assets* assets) :
		_assetData(assetHandle, assets)
	{
		if (ov_open_callbacks(&_assetData, &_vf, nullptr, 0, readCallbacks) < 0)
		{
			throw std::runtime_error("Failed to create vorbis decoder");
		}

		vorbis_info* vi = ov_info(&_vf, -1);

		_channels = vi->channels;
		_rate = vi->rate;
		_totalSampleCount = ov_pcm_total(&_vf, -1);
	}

	void Vorbis_Decoder::Decode(int size, uint8_t* output)
	{
		int currentSection;
		long readBytes = ov_read(&_vf, reinterpret_cast<char*>(output), size, 0, 2, 1, &currentSection);

		if (readBytes == 0)
		{
			throw std::runtime_error("End of file!");
		}
		else if (size != readBytes)
		{
			throw std::runtime_error("Size != readBytes");
		}
	}

	Vorbis_Decoder::~Vorbis_Decoder()
	{
		ov_clear(&_vf);
	}

	static size_t Read(void* output, size_t size, size_t amount, void* assetData)
	{
		if (size * amount == 0)
			return 0;

		auto [handle, assets] = *reinterpret_cast<AssetData*>(assetData);

		int bytesRead  = assets->ReadBytes(handle, output, size * amount);
		int readAmount = bytesRead / size;

		return readAmount;
	}

	static int Seek(void* assetData, ogg_uint64_t offset, int whence)
	{
		auto [handle, assets] = *reinterpret_cast<AssetData*>(assetData);

		filesystem::FileSeekDir dir;

		switch (whence) {
			case SEEK_CUR: dir = filesystem::FileSeekDir::Cur; break;
			case SEEK_SET: dir = filesystem::FileSeekDir::Beg; break;
			case SEEK_END: dir = filesystem::FileSeekDir::End; break;
		}

		assets->Seek(handle, offset, dir);

		return 0;
	}

	static long Tell(void* assetData)
	{
		auto [handle, assets] = *reinterpret_cast<AssetData*>(assetData);

		return assets->GetPosition(handle);
	}
}