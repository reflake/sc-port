#pragma once

#include <SDL2/SDL_video.h>
#include <SDL_surface.h>

#include <filesystem/Storage.hpp>
#include <memory>
#include <vector>
#include <vulkan/vulkan_core.h>

#include "../A_Graphics.hpp"
#include "RenderPass.hpp"
#include "Shader.hpp"
#include "Window.hpp"
#include "data/Assets.hpp"
#include "data/Common.hpp"
#include "data/Sprite.hpp"
#include "device/Device.hpp"
#include "device/SwapChain.hpp"
#include "memory/BufferAllocator.hpp"
#include "Vertex.hpp"

namespace renderer::vulkan
{
	extern int DeviceEvaluation(VkPhysicalDevice&);

	extern const std::vector<const char*> validationLayers;

	class Graphics : public A_Graphics
	{
	public:

		Graphics(SDL_Window* window, const data::Assets* assets);

		const A_Drawable* LoadSpriteSheet(data::A_SpriteSheetData&) override;
		const A_Drawable* LoadTileset(data::A_TilesetData&) override;

		void Draw(const A_Drawable*, frameIndex, data::position) override;
		void FreeDrawable(const A_Drawable*) override;

		void SetTilesetPalette(data::Palette&) override;

		void SetView(data::position pos) override;
		void BeginRendering() override;
		void PresentToScreen() override;

		const char* GetName() const override { return "Vulkan"; }

		void WaitIdle() override;

		void Release() override;

	private:

		void CreateInstance(std::vector<const char*>& enabledLayers);
		void EnableValidationLayers(std::vector<const char*>& layerList);
		void CreateSyncObjects();
		void Submit();
		void Present();

		ShaderCode ReadShaderCode(const char* path);

	private:

		// TODO: hide these members in implementation
		const data::Assets*  _assets;

		Config          _config;
		VkInstance      _instance;
		VkSurfaceKHR    _surface;
		VkCommandPool   _commandPool;
		VkCommandBuffer _commandBuffer;
		Swapchain       _swapchain;
		Device          _device;
		Window          _window;
		RenderPass      _renderPass;
		ShaderManager   _shaders;
		BufferAllocator _bufferAllocator;

		VkSemaphore _imageAvailableSemaphore, _renderFinishedSemaphore;
		VkFence     _fence;

		data::position _currentPosition;
		uint32_t _currentImageIndex;

		uint32_t   _mainShaderIndex;

		std::vector<Vertex> _vertexBuffer;
	};
}