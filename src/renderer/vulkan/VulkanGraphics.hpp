#pragma once

#include <SDL2/SDL_video.h>
#include <SDL_surface.h>

#include <filesystem/Storage.hpp>
#include <vulkan/vulkan_core.h>

#include "../A_Graphics.hpp"

namespace renderer::vulkan
{
	extern int DeviceEvaluation(VkPhysicalDevice&);

	class Graphics : public A_Graphics
	{
	public:

		Graphics(SDL_Window* window, filesystem::Storage& storage);

		void LoadGrp(grpID) override;
		void FreeGrp(grpID) override;
		void DrawGrpFrame(grpID grpID, uint32_t frame, glm::vec2 position) override;

		void LoadTileset(data::Tileset) override;
		void DrawTile(data::Tileset, tileID, glm::vec2 position) override;
		void FreeTileset(data::Tileset) override;

		void SetTilesetPalette(data::Tileset) override;

		void CycleWaterPalette() override;

		void ClearDepth() override;
		void PresentToScreen() override;

		const char* GetName() const override { return "Vulkan"; }

	private:

		void CreateInstance();
		void EnableValidationLayers(VkInstanceCreateInfo&);

	private:

		VkInstance    _instance;
		VkSurfaceKHR  _surface;

		SDL_Window* const _window;

		const filesystem::Storage& _storage;
	};
}