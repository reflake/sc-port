#pragma once 

#include <cstdint>
#include <glm/detail/qualifier.hpp>

namespace renderer
{
	class SpriteFrameData
	{
	public:

		glm::vec<2, uint32_t> GetDimensions();
		glm::vec<2, uint32_t> GetOffset();
		void* GetPixelData();

	private:


	};
};