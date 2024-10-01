#pragma once

#include "../Graphics.hpp"

namespace renderer::vulkan
{
	class Graphics : public A_Graphics
	{
	public:

		void LoadGrp(grpID) override;
		void DrawGrpFrame(grpID grpID, uint32_t frame, glm::vec2 position) override;

		void CycleWaterPalette() override;

		const char* GetName() const override { return "Vulkan"; }
	};
}