#include "WebmReader.hpp"

namespace video
{
	webm::Status WebmReader::Read(std::size_t size, std::uint8_t* buffer, std::uint64_t* bytesRead)
	{
		*bytesRead = _assets->ReadBytes(_videoFileHandle, buffer, size);

		if (*bytesRead == 0)
		{
			return webm::Status(webm::Status::kEndOfFile);
		}

		return webm::Status(*bytesRead < size ? webm::Status::kOkPartial : webm::Status::kOkCompleted);
	}
	
	webm::Status WebmReader::Skip(std::uint64_t bytesToSkip, std::uint64_t* actuallySkipped)
	{
		int lastPosition = _assets->GetPosition(_videoFileHandle);

		_assets->Seek(_videoFileHandle, bytesToSkip, filesystem::FileSeekDir::Cur);

		*actuallySkipped = _assets->GetPosition(_videoFileHandle) - lastPosition;

		if (*actuallySkipped == 0)
		{
			return webm::Status(webm::Status::kEndOfFile);
		}

		return webm::Status(*actuallySkipped < bytesToSkip ? webm::Status::kOkPartial : webm::Status::kOkCompleted);
	}

	std::uint64_t WebmReader::Position() const
	{
		return _assets->GetPosition(_videoFileHandle);
	}
};