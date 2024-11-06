#pragma once

#include <cstdint>
#include <glm/vec2.hpp>
#include <memory>
#include <vector>

#include "../filesystem/Storage.hpp"

#include <data/Sprite.hpp>

namespace data
{
	
	const int GRP_DIMENSIONS_LIMIT = 256;
	const int GRP_SPRITE_SQUARE_LIMIT = GRP_DIMENSIONS_LIMIT * GRP_DIMENSIONS_LIMIT;

	struct GrpHeader
	{
		uint16_t              frameAmount;
		glm::vec<2, uint16_t> dimensions;
	};

	struct GrpFrame
	{
		glm::vec<2, uint8_t> posOffset;
		glm::vec<2, uint8_t> dimensions;
		uint32_t             linesOffset;
	};

	class Grp : public A_SpriteSheetData
	{
	public:

		// Overridden members
		const SpriteData GetSpriteData(int frame) const override;
		const int        ReadPixelData(int frame, uint8_t* out) const override;

		glm::vec<2, int> GetDimensionsLimit() const override;

		// concrete members
		const GrpHeader& GetHeader() const;
		const GrpFrame&  GetFrame(int frame) const;
		const std::vector<GrpFrame>& GetFrames() const;

		static void ReadGrpFile(filesystem::Storage& storage, const char* path, Grp& grp);
		static void ReadGrp(std::shared_ptr<uint8_t[]> data, int size, Grp& grp);

	private:

		GrpHeader                  _header;
		std::vector<GrpFrame>      _frames;
		std::shared_ptr<uint8_t[]> _data;
	};
}