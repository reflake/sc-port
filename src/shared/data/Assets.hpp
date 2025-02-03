#pragma once

#include "filesystem/Storage.hpp"
#include <SDL_rwops.h>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <typeinfo>

#include <boost/any.hpp>

namespace data
{
	typedef void* AssetHandle;

	struct PreloadedResource
	{
		std::string path;
		std::string typeName;
		boost::any  data;
		bool        isBinary = false;
	};

	
	template<typename T, typename = void>
	struct has_load : std::false_type {};

	template<typename T>
	struct has_load<T, std::void_t<decltype(std::declval<T>().Load(std::declval<std::shared_ptr<uint8_t[]>>(), std::declval<uint32_t>()))>> : std::true_type {};

	class Assets
	{
	public:

		Assets();
		Assets(filesystem::Storage*);

		template<typename T, typename std::enable_if<has_load<T>::value, int>::type = 0>
		void Preload(const char* path)
		{
			AssetHandle asset = Open(path);

			int rawDataSize = GetSize(asset);
			auto tmpRawData = std::make_shared<uint8_t[]>(rawDataSize);

			ReadBytes(asset, tmpRawData.get(), rawDataSize);
			Close(asset);


			T* preloadedObject = new T();
			preloadedObject->Load(tmpRawData, rawDataSize);

			auto typeName = typeid(T).name();

			Preload(path, preloadedObject, typeName, false);
		}

		template<typename T, typename std::enable_if<!has_load<T>::value, int>::type = 0>
		void Preload(const char* path)
		{
			auto typeName = typeid(T).name();
			auto data = std::make_shared<uint8_t[]>(sizeof(T));

			ReadBytes(path, data.get());

			Preload(path, data, typeName, true);
		}

		template<typename T>
		T* Get(const char* path) const
		{
			auto typeName = typeid(T).name();

			for(auto& resource : _preloadedResources)
			{
				if (resource.path == path && resource.typeName == typeName)
				{
					if (resource.isBinary)
					{
						auto binaryData = boost::any_cast<std::shared_ptr<uint8_t[]>>(resource.data).get();

						return reinterpret_cast<T*>(binaryData);
					}
					else
					{
						return boost::any_cast<T*>(resource.data);
					}
					break;
				}
			}

			throw std::runtime_error("Unexpected type");
		}

		template<typename T>
		void Read(const char* path, T& output) const
		{
			auto data = reinterpret_cast<uint8_t*>(&output);

			ReadBytes(path, data);
		}

		template<typename T>
		int ReadBytes(AssetHandle asset, T* output, int size) const
		{
			uint8_t* data = reinterpret_cast<uint8_t*>(output);

			return ReadBytes(asset, data, size);
		}

		void Preload(const char* path, boost::any data, const char* typeName, bool binary);

		int ReadBytes(const char* path, uint8_t* output) const;
		int GetSize(const char* path) const;
		
		AssetHandle Open(const char* path);
		int  ReadBytes(AssetHandle, uint8_t* output, int size) const;
		void Seek(AssetHandle, int offset, filesystem::FileSeekDir dir);
		int  GetSize(AssetHandle) const;
		int  GetPosition(AssetHandle) const;
		bool IsEOF(AssetHandle) const;
		void Close(AssetHandle);

		void AssetToSdlReadIO(SDL_RWops*, AssetHandle);

	private:

		filesystem::Storage* _storage;

		std::vector<PreloadedResource> _preloadedResources;
	};
}