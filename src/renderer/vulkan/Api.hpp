#pragma once

#include "../GraphicAPI.hpp"

namespace renderer::vulkan
{
	class Api : public GraphicAPI
	{
	public:

		void LoadGrp(grpID) override;
		void DrawGrpFrame(grpID grpID, uint32_t frame, glm::vec2 position) override;

		void CycleWaterPalette() override;

		const char* GetName() const override { return "Vulkan"; }
	};
}