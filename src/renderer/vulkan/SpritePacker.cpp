#include "SpritePacker.hpp"
#include "data/Sprite.hpp"

#include <cmath>
#include <rectpack2D/finders_interface.h>
#include <vector>

using namespace rectpack2D;

namespace renderer::vulkan
{

	SpritePacker::SpritePacker(const std::vector<data::SpriteData>& frameDatas) 
		: _frameDatas(frameDatas)
		{}

	Atlas SpritePacker::CreateAtlas()
	{
		using spacesType = empty_spaces<false, default_empty_spaces>;
		using rectType   = output_rect_t<spacesType>;

		const int MAX_SIDE_SIZE = 1024;
		const int MAX_STEPS = 5;

		std::vector<rectType> rectangles;

		for(auto& frame : _frameDatas)
		{
			rectangles.push_back(rect_xywh(0, 0, frame.dimensions.x, frame.dimensions.y));
		}

		auto report_successful = [](rectType&) {
			return callback_result::CONTINUE_PACKING;
		};

		auto report_unsuccessful = [](rectType&) {
			return callback_result::ABORT_PACKING;
		};

		auto [width, height] = find_best_packing<spacesType>(rectangles, make_finder_input(MAX_SIDE_SIZE, MAX_STEPS, report_successful, report_unsuccessful, flipping_option::DISABLED));

		std::vector<data::SpriteRect> outputRects;

		for(auto& frame : rectangles)
		{
			outputRects.emplace_back(frame.x, frame.y, frame.w, frame.h);
		}

		width = pow(2, ceil(log2(width)));
		height = pow(2, ceil(log2(height)));

		return Atlas(width, height, std::move(outputRects));
	}
}