#include "VulkanGraphics.hpp"

#include "Command.hpp"
#include "Config.hpp"
#include "Drawable.hpp"
#include "Shader.hpp"
#include "data/Assets.hpp"
#include "data/Common.hpp"
#include "data/Palette.hpp"
#include "data/Tileset.hpp"
#include "device/Device.hpp"
#include "Window.hpp"
#include "device/Format.hpp"
#include "device/Queue.hpp"
#include "device/SwapChain.hpp"
#include "memory/BufferAllocator.hpp"

#include <algorithm>
#include <cstdint>
#include <glm/ext/matrix_transform.hpp>
#include <memory>
#include <stdexcept>
#include <vector>

#include <vulkan/vulkan_core.h>

#include <glm/detail/qualifier.hpp>
#include <glm/vec2.hpp>

namespace renderer::vulkan
{
	using glm::vec2;
	using std::array;
	using std::runtime_error;
	using std::vector;
	using ShaderStage = ShaderModule::Stage;

	const std::vector<const char*> validationLayers = {
		"VK_LAYER_KHRONOS_validation"
	};

	Graphics::Graphics(SDL_Window* window, const data::Assets* assets) : _window(window), _assets(assets)
	{
		const VkAllocationCallbacks* allocator = VK_NULL_HANDLE;

		// Check out layers
		vector<const char*> requiredLayers;

		if (enableValidationLayers)
		{
			if (!CheckValidationLayerSupported())
				throw runtime_error("Validation layer is not supported");

			EnableValidationLayers(requiredLayers);
		}

		CreateInstance(requiredLayers);
		CreateWindowSurface(window, _instance, _surface);

		auto [screenWidth, screenHeight] = _window.GetExtent();

		_config = Config(screenWidth, screenHeight);

		auto physicalDevice = PickPhysicalDevice(_instance, _surface, &DeviceEvaluation);

		// Only one main device is being used
		_device     = Device::Create(physicalDevice, _surface, requiredLayers);

		auto surfaceFormat = PickSwapSurfaceFormat(physicalDevice, _surface);

		_renderPass = RenderPass::Create(_device, surfaceFormat.format);
		_swapchain  = Swapchain::Create(_device, &_renderPass, _surface, surfaceFormat, _config);
		_shaders    = ShaderManager(&_device, &_config, &_renderPass);

		auto fragmentShaderModule = _shaders.CreateShaderModule(ShaderStage::Fragment, ReadShaderCode("shaders/frag.spv"));
		auto vertexShaderModule   = _shaders.CreateShaderModule(ShaderStage::Vertex, ReadShaderCode("shaders/vert.spv"));
		array<uint32_t, 2> modules = { fragmentShaderModule, vertexShaderModule };

		_mainShaderIndex = _shaders.CreateShader(modules.data(), modules.size(), _swapchain.GetFormat());

		QueueFamilyIndices familyIndices = FindQueueFamilies(_device, _surface);
		_commandPool = CreateCommandPool(_device, familyIndices.graphicsFamily.value());
		_commandBuffer = CreateCommandBuffer(_device, _commandPool);

		CreateSyncObjects();

		_bufferAllocator = BufferAllocator(&_device, allocator);
		_bufferAllocator.Initialize();
	}

	void Graphics::CreateInstance(vector<const char*>& enabledLayers)
	{
		std::vector<const char*> extensions;

		GetSdlRequiredExtensions(_window, extensions);

		const char* appName = "gauss-app";
		const char* engineName = "gauss-engine";
		const int   appVersion = 0;
		const int   engineVersion = 0;

		VkApplicationInfo appInfo(
			VK_STRUCTURE_TYPE_APPLICATION_INFO, nullptr, 
			appName, appVersion, engineName, engineVersion, VK_API_VERSION_1_0);

		const VkInstanceCreateFlags flags = 0;

		VkInstanceCreateInfo instanceCreateInfo(
			VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO, nullptr,
			flags, &appInfo, 
			enabledLayers.size(), enabledLayers.data(),
			extensions.size(), extensions.data());

		auto errCode = vkCreateInstance(&instanceCreateInfo, nullptr, &_instance);

		if (errCode != VK_SUCCESS)
		{
			switch(errCode)
			{
				case VK_ERROR_LAYER_NOT_PRESENT:
					throw runtime_error("Failed to create Vulkan API instance: cannot enable validation error");
					break;
				default:
					throw runtime_error("Failed to create Vulkan API instance");
					break;
			}
		}
	}

	void Graphics::EnableValidationLayers(std::vector<const char*>& layerList)
	{
		for(auto layerName : validationLayers)
		{
			layerList.emplace_back(layerName);
		}
	}

	void Graphics::CreateSyncObjects()
	{
		const VkAllocationCallbacks* allocator = nullptr;

		VkSemaphoreCreateInfo semaphoreCreateInfo { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
		VkFenceCreateInfo     fenceCreateInfo { VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };

		// must be signaled initially so the program won't halt on the first frame
		fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		if (vkCreateSemaphore(_device, &semaphoreCreateInfo, allocator, &_imageAvailableSemaphore) != VK_SUCCESS || 
				vkCreateSemaphore(_device, &semaphoreCreateInfo, allocator, &_renderFinishedSemaphore) != VK_SUCCESS ||
				vkCreateFence(_device, &fenceCreateInfo, allocator, &_fence) != VK_SUCCESS)
		{
			throw runtime_error("Failed to create synchronization objects");
		}
	}

	int DeviceEvaluation(VkPhysicalDevice& device)
	{
		int score = 0;

		VkPhysicalDeviceProperties props;
		vkGetPhysicalDeviceProperties(device, &props);

		if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
		{
			score += 2000;
		}

		// score += props.limits.maxImageDimension2D;
		score += props.limits.maxMemoryAllocationCount / 20000;

		return score;
	}
	
	DrawableHandle Graphics::LoadSpriteSheet(data::A_SpriteSheetData& spriteSheetData)
	{
	/* Примерный алгоритм действий для чтения Grp спрайтов

		for(auto& frame : grp.GetFrames())
		{
			SDL_Surface* surface = SDL_CreateRGBSurface(0, frame.dimensions.x, frame.dimensions.y, 8, 0, 0, 0, 0);
			SDL_SetSurfacePalette(surface, app.tilesetAtlas.palette);

			SDL_LockSurface(surface);

			grp.GetFramePixels(frameIndex, pixels);

			int width = frame.dimensions.x;
			auto surfacePixels = reinterpret_cast<uint8_t*>(surface->pixels);

			for(int i = 0; i < surface->h; i++)
			{
				memcpy(surfacePixels + i * surface->pitch, pixels + i * width, width);
			}

			SDL_UnlockSurface(surface);
			SDL_SetColorKey(surface, SDL_TRUE, 0x00000000);

			atlas.rects.push_back(SDL_Rect { 
				.x = frame.posOffset.x - grp.GetHeader().dimensions.x / 2, .y = frame.posOffset.y - grp.GetHeader().dimensions.y / 2,
				.w = frame.dimensions.x, .h = frame.dimensions.y });
			atlas.surfaces[frameIndex++] = surface;
		}

		app.spriteAtlases[doodad->grpID] = atlas;*/

		return nullptr; // TODO:
	}

	DrawableHandle Graphics::LoadTileset(data::A_TilesetData& tilesetData)
	{
		Tileset* tileset = new Tileset(tilesetData.GetTileSize());

		for(int i = 0; i < tilesetData.GetTileCount(); i++)
		{

		}

		_drawables.push_back(tileset);

		return tileset;
	}

	void Graphics::Draw(DrawableHandle drawable, frameIndex frame, data::position position)
	{
		// Break into another draw call
		if (drawable != _currentDrawable)
		{
			if (!_vertexBuffer.empty())

				DrawStreamVertexBuffer();

			// Validate new drawable
			auto vulkanDrawable = reinterpret_cast<A_VulkanDrawable*>(drawable);

			// TODO: search drawables from cache
			if (std::find(_drawables.begin(), _drawables.end(), vulkanDrawable) == _drawables.end())
			{
				throw runtime_error("Incompatible drawable object");
			}

			_currentDrawable = vulkanDrawable;
		}

		array<Vertex, 10> polygonVertices;

		auto size = _currentDrawable->GetPolygon(frame, polygonVertices, polygonVertices.size());

		if (size > polygonVertices.size())
		{
			throw runtime_error("Too much polygons");
		}

		for(int i = 0; i < size; i++)
		{
			auto& vertex = polygonVertices[i];

			vertex.pos   += position - _currentPosition;
			vertex.pos.x /= _config.GetExtents().width;
			vertex.pos.y /= _config.GetExtents().height;

			vertex.pos = vertex.pos * 2.0f - 1.0f;
		}

		for(int i = 0; i < size; i++)
		{
			_vertexBuffer.push_back(polygonVertices[i]);
		}
	}

	void Graphics::DrawStreamVertexBuffer()
	{
		auto streamData = _bufferAllocator.WriteToStreamBuffer(sizeof(Vertex) * _vertexBuffer.size(), _vertexBuffer.data());

		streamData.BindToCommandBuffer(_commandBuffer);
		vkCmdDraw(_commandBuffer, _vertexBuffer.size(), 1, 0, 0);

		_vertexBuffer.clear();
	}

	void Graphics::FreeDrawable(DrawableHandle)
	{
		// TODO:
	}

	void Graphics::SetTilesetPalette(data::Palette&)
	{
		// TODO:
	}

	void Graphics::SetView(data::position pos)
	{
		_currentPosition = pos;
	}

	void Graphics::BeginRendering()
	{
		// Wait for previous frame to be rendered
		vkWaitForFences(_device, 1, &_fence, VK_TRUE, UINT64_MAX);
		vkResetFences(_device, 1, &_fence);

		vkResetCommandBuffer(_commandBuffer, 0);
		
		// ==============================================
		VkCommandBufferBeginInfo beginInfo { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };

		beginInfo.flags = 0;
		beginInfo.pInheritanceInfo = nullptr;

		if (vkBeginCommandBuffer(_commandBuffer, &beginInfo) != VK_SUCCESS)
		{
			throw runtime_error("Failed to begin recording command buffer");
		}

		// Start render pass
		VkRenderPassBeginInfo renderBeginInfo { VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };

		_currentImageIndex = _swapchain.GetNextImageIndex(_imageAvailableSemaphore);

		renderBeginInfo.renderPass = _renderPass;
		renderBeginInfo.framebuffer = _swapchain.GetFrameBuffer(_currentImageIndex);
		renderBeginInfo.renderArea.offset = {0, 0};
		renderBeginInfo.renderArea.extent = _config.GetExtents();

		VkClearValue clearColor = {{{0.66, 0.66, 0.66, 1}}};
		renderBeginInfo.clearValueCount = 1;
		renderBeginInfo.pClearValues = &clearColor;

		vkCmdBeginRenderPass(_commandBuffer, &renderBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		// Bind main pipeline and prepare viewport
		auto pipeline = _shaders.GetShaderPipeline(_mainShaderIndex);

		vkCmdBindPipeline(_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

		auto [width, height] = _config.GetExtents();

		VkViewport viewport { 
			0.0, 0.0, 
			static_cast<float>(width), static_cast<float>(height), 
			0.0f, 1.0f};

		vkCmdSetViewport(_commandBuffer, 0, 1, &viewport);

		VkRect2D scissor { 
			0, 0,
			_config.GetExtents() };

		vkCmdSetScissor(_commandBuffer, 0, 1, &scissor);

		// ==============================================

		_bufferAllocator.OnBeginRendering();

		_vertexBuffer.clear();

		_currentDrawable = nullptr;
	}

	void Graphics::PresentToScreen()
	{
		if (!_vertexBuffer.empty())

			DrawStreamVertexBuffer();

		_bufferAllocator.OnPrepareForPresentation();

		vkCmdEndRenderPass(_commandBuffer);

		if (vkEndCommandBuffer(_commandBuffer) != VK_SUCCESS)
		{
			throw runtime_error("Failed to record command buffer");
		}

		Submit();
		Present();
	}

	void Graphics::Submit()
	{
		VkSubmitInfo submitInfo { VK_STRUCTURE_TYPE_SUBMIT_INFO };

		array<VkSemaphore, 1> waitSemaphores = { _imageAvailableSemaphore };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

		submitInfo.waitSemaphoreCount = waitSemaphores.size();
		submitInfo.pWaitSemaphores = waitSemaphores.data();
		submitInfo.pWaitDstStageMask = waitStages;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &_commandBuffer;

		array<VkSemaphore, 1> signalSemaphores = { _renderFinishedSemaphore };
		submitInfo.signalSemaphoreCount = signalSemaphores.size();
		submitInfo.pSignalSemaphores = signalSemaphores.data();

		if (vkQueueSubmit(_device.GetGraphicsQueue(), 1, &submitInfo, _fence) != VK_SUCCESS)
		{
			throw runtime_error("Failed to submit draw command buffer");
		}
	}

	void Graphics::Present()
	{
		VkPresentInfoKHR presentInfo { VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };

		array<VkSemaphore, 1> waitSemaphores = { _renderFinishedSemaphore };

		presentInfo.waitSemaphoreCount = waitSemaphores.size();
		presentInfo.pWaitSemaphores = waitSemaphores.data();

		const uint32_t swapchainCount = 1;

		array<VkSwapchainKHR, swapchainCount> swapchains = { _swapchain };
		array<uint32_t, swapchainCount> imageIndices = { _currentImageIndex };

		presentInfo.swapchainCount = swapchains.size();
		presentInfo.pSwapchains = swapchains.data();
		presentInfo.pImageIndices = imageIndices.data();

		presentInfo.pResults = VK_NULL_HANDLE;

		vkQueuePresentKHR(_device.GetPresentQueue(), &presentInfo);
	}

	ShaderCode Graphics::ReadShaderCode(const char* path)
	{
		ShaderCode output;
		output.size = _assets->GetSize(path);
		output.code = std::make_unique<uint8_t[]>(output.size);

		_assets->ReadBytes(path, output.code.get());

		return output;
	}

	void Graphics::WaitIdle()
	{
		vkDeviceWaitIdle(_device);
	}

	void Graphics::Release()
	{
		const VkAllocationCallbacks* const allocator = nullptr;

		for(auto drawable : _drawables)
		{
			switch(drawable->GetType())
			{
				case SpriteSheetType:
					delete dynamic_cast<SpriteSheet*>(drawable);
					break;

				case TilesetType:
					delete dynamic_cast<Tileset*>(drawable);
					break;
			}
		}

		_bufferAllocator.Release();

		if (_imageAvailableSemaphore)
			vkDestroySemaphore(_device, _imageAvailableSemaphore, allocator);

		if (_renderFinishedSemaphore)
			vkDestroySemaphore(_device, _renderFinishedSemaphore, allocator);

		if (_fence)
			vkDestroyFence(_device, _fence, allocator);

		if (_commandBuffer)
			vkFreeCommandBuffers(_device, _commandPool, 1, &_commandBuffer);

		_commandBuffer = nullptr;

		if (_commandPool)
			vkDestroyCommandPool(_device, _commandPool, allocator);

		_commandPool = nullptr;

		_shaders.Destroy();
		
		_renderPass.Destroy();

		_swapchain.Destroy();

		_device.Destroy();

		if (_surface)
			vkDestroySurfaceKHR(_instance, _surface, allocator);

		_surface = nullptr;

		if (_instance)
			vkDestroyInstance(_instance, allocator);

		_instance = nullptr;
	}

	/*void Graphics::CycleWaterPalette()
	{
		 примерный алгоритм действий
		
		cyclePaletteColor<1, 6>(app.tilesetAtlas);
		cyclePaletteColor<7, 7>(app.tilesetAtlas);
		
	}*/
}