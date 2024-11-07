#pragma once

#include <SDL2/SDL_video.h>
#include <SDL_surface.h>

#include <filesystem/Storage.hpp>
#include <vulkan/vulkan_core.h>

#include "../A_Graphics.hpp"
#include "data/Sprite.hpp"
#include "device/Device.hpp"
#include "Window.hpp"

namespace renderer::vulkan
{
	extern int DeviceEvaluation(VkPhysicalDevice&);

	extern const std::vector<const char*> validationLayers;

	class Graphics : public A_Graphics
	{
	public:

		Graphics(SDL_Window* window, filesystem::Storage& storage);
		~Graphics();

		const A_SpriteSheet* LoadSpriteSheet(data::A_SpriteSheetData&) override;
		void DrawSprite(const A_SpriteSheet*, frameIndex, glm::vec2 position) override;
		void FreeSpriteSheet(const A_SpriteSheet*) override;

		const A_Tileset* LoadTileset(data::A_TilesetData&) override;
		void DrawTile(const A_Tileset*, tileID, glm::vec2 position) override;
		void FreeTileset(const A_Tileset*) override;

		void SetTilesetPalette(data::Palette) override;

		void ClearDepth() override;
		void PresentToScreen() override;

		const char* GetName() const override { return "Vulkan"; }

	private:

		void CreateInstance();
		void EnableValidationLayers(std::vector<const char*>& layerList);

	private:

		VkInstance    _instance;
		VkSurfaceKHR  _surface;
		Device        _device;

		Window _window;

		const filesystem::Storage& _storage;
	};
}