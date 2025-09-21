#pragma once

#include <SDL3/SDL_video.h>
#include <SDL3/SDL_surface.h>

#include <array>
#include <filesystem/Storage.hpp>
#include <memory>
#include <vector>
#include <vulkan/vulkan_core.h>

#include "../A_Graphics.hpp"
#include "DescriptorSetLayout.hpp"
#include "Drawable.hpp"
#include "RenderPass.hpp"
#include "Sampler.hpp"
#include "Shader.hpp"
#include "Window.hpp"
#include "data/Assets.hpp"
#include "data/Common.hpp"
#include "data/Sprite.hpp"
#include "device/Device.hpp"
#include "device/SwapChain.hpp"
#include "memory/BufferAllocator.hpp"
#include "Vertex.hpp"
#include "memory/MemoryManager.hpp"
#include "Sampler.hpp"
#include "DescriptorSetLayout.hpp"

namespace renderer::vulkan
{
	extern int DeviceEvaluation(VkPhysicalDevice&);

	extern const std::vector<const char*> validationLayers;

	const uint32_t POOL_MAX_SETS = 200;

	struct DrawCall
	{
		A_VulkanDrawable* drawable = nullptr;
		uint32_t          vertexCount = 0;
		StreamData        streamData;
	};

	class Graphics : public A_Graphics
	{
	public:

		Graphics(SDL_Window* window, const data::Assets* assets);

		DrawableHandle LoadSpriteSheet(data::A_SpriteSheetData&) override;
		DrawableHandle LoadTileset(data::A_TilesetData&, std::vector<bool>& usedTiles) override;
		DrawableHandle LoadImage(uint32_t* pixels, uint32_t width, uint32_t height) override;

		void Draw(DrawableHandle, frameIndex, data::position) override;
		void Draw(DrawableHandle, data::position, uint32_t width, uint32_t height) override;
		void FreeDrawable(DrawableHandle) override;

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
		void CreateDescriptorPools();
		DrawCall* UseDrawCall(DrawableHandle);
		void ClearDescriptorPool();
		void AllocateDescriptorSets();
		void WriteDescriptorSets();
		void Submit();
		void Present();

		A_VulkanDrawable* ConvertToDrawable(DrawableHandle);
		std::vector<A_VulkanDrawable*>::iterator FindDrawable(DrawableHandle);

		ShaderCode ReadShaderCode(const char* path);

	private:

		// TODO: hide these members in implementation
		const data::Assets*  _assets;

		Config              _config;
		VkInstance          _instance;
		VkSurfaceKHR        _surface;
		VkCommandPool       _commandPool;
		VkCommandBuffer     _commandBuffer;
		Swapchain           _swapchain;
		Device              _device;
		Window              _window;
		RenderPass          _renderPass;
		ShaderManager       _shaders;
		BufferAllocator     _bufferAllocator;
		MemoryManager       _memoryManager;
		DescriptorSetLayout _standardLayout, _samplerLayout;
		VkDescriptorPool    _descriptorPool;
		VkDescriptorSet     _descriptorSets[POOL_MAX_SETS];
		int                 _descriptorSetCount = 0;

		Image*                _tilesetImage;
		std::vector<uint32_t> _tileMap;

		VkSemaphore _imageAvailableSemaphore, _renderFinishedSemaphore;
		VkFence     _fence;

		data::position _currentPosition;
		uint32_t       _currentImageIndex;
		uint32_t       _mainShaderIndex, _textureShaderIndex;

		DrawCall* _currentDrawCall;

		Sampler _textureSampler, _textureLinearInterpSampler;

		std::vector<A_VulkanDrawable*>    _drawables;
		std::array<A_VulkanDrawable*, 4>  _drawablesCache;
		uint32_t                          _drawablesCacheIndex = 0;

		std::vector<DrawCall>          _drawCalls;
	};
}