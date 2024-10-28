#pragma once 

#include <cstdint>
#include <glm/vec2.hpp>
#include <memory>

namespace renderer
{
	class SpriteFrameData
	{
	public:

		SpriteFrameData(std::shared_ptr<uint8_t> pixelData, glm::vec<2, uint32_t> dimensions, glm::vec<2, uint32_t> offset);

		glm::vec<2, uint32_t> GetDimensions() const;
		glm::vec<2, uint32_t> GetOffset() const;
		const void* GetPixelData() const;

	private:

		glm::vec<2, uint32_t> _dimensions;
		glm::vec<2, uint32_t> _offset;
		std::shared_ptr<uint8_t> _pixelData;
	};
};