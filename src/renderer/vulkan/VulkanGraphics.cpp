#include "VulkanGraphics.hpp"

#include "Command.hpp"
#include "Config.hpp"
#include "Shader.hpp"
#include "data/Assets.hpp"
#include "data/Common.hpp"
#include "device/Device.hpp"
#include "Window.hpp"
#include "device/Queue.hpp"
#include "device/SwapChain.hpp"

#include <cstdint>
#include <glm/vec2.hpp>
#include <memory>
#include <stdexcept>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace renderer::vulkan
{
	using glm::vec2;
	using std::array;
	using std::runtime_error;
	using std::vector;
	using ShaderStage = ShaderModule::Stage;

	const float unitsPerPixel = 1 / 256.0f;

	Graphics::Graphics(SDL_Window* window, const data::Assets* assets) : _window(window), _assets(assets)
	{
		// Check out layers
		vector<const char*> requiredLayers;

		if (enableValidationLayers)
		{
			if (!CheckValidationLayerSupported(_device))
				throw runtime_error("Validation layer is not supported");

			EnableValidationLayers(requiredLayers);
		}

		CreateInstance(requiredLayers);
		CreateWindowSurface(window, _instance, _surface);

		auto [screenWidth, screenHeight] = _window.GetExtent();

		_config = Config(screenWidth, screenHeight);

		auto physicalDevice = PickPhysicalDevice(_instance, &DeviceEvaluation);

		// Only one main device is being used
		_device     = Device::Create(physicalDevice, _surface, requiredLayers);
		_renderPass = RenderPass::Create(_device, _swapchain.GetFormat());
		_swapchain  = Swapchain::Create(_device, &_renderPass, _surface, _config);
		_shaders    = ShaderManager(&_device, &_config, &_renderPass);

		auto fragmentShaderModule = _shaders.CreateShaderModule(ShaderStage::Fragment, ReadShaderCode("shader.frag"));
		auto vertexShaderModule   = _shaders.CreateShaderModule(ShaderStage::Vertex, ReadShaderCode("shader.vert"));
		array<const ShaderModule*, 2> modules = { fragmentShaderModule, vertexShaderModule };

		_mainShader = _shaders.CreateShader(modules.data(), modules.size(), _swapchain.GetFormat());

		QueueFamilyIndices familyIndices = FindQueueFamilies(_device, _surface);
		_commandPool = CreateCommandPool(_device, familyIndices.graphicsFamily.value());
		_commandBuffer = CreateCommandBuffer(_device, _commandPool);

		CreateSyncObjects();
	}

	void Graphics::CreateInstance(vector<const char*> enabledLayers)
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

		score += props.limits.maxImageDimension2D;
		score += props.limits.maxMemoryAllocationCount;

		return score;
	}
	
	const A_SpriteSheet* Graphics::LoadSpriteSheet(data::A_SpriteSheetData& spriteSheetData)
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
	}

	void Graphics::DrawGrpFrame(grpID grpID, uint32_t frame, vec2 position)
	{
		/* Примерный алгоритм отрисовки спрайта на экран
		
		auto spriteAtlas = spriteAtlases[grpID];
		auto surface     = spriteAtlas.surfaces[frame];
		auto destRect    = spriteAtlas.rects[frame];

		destRect.x += doodad->pos.x;
		destRect.y += doodad->pos.y;

		SDL_BlitSurface(surface, nullptr, app.screenSurface, &destRect);*/
	}

	void Graphics::SetView(data::position pos)
	{
		_currentPosition = { pos.x * unitsPerPixel, pos.y * unitsPerPixel };
	}

	void Graphics::BeginRendering()
	{
		// Wait for previous frame to be rendered
		vkWaitForFences(_device, 1, &_fence, VK_TRUE, UINT64_MAX);
		vkResetFences(_device, 1, &_fence);

		vkResetCommandBuffer(_commandBuffer, 0);

		VkCommandBufferBeginInfo beginInfo { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };

		beginInfo.flags = 0;
		beginInfo.pInheritanceInfo = nullptr;

		if (vkBeginCommandBuffer(_commandBuffer, &beginInfo) != VK_SUCCESS)
		{
			throw runtime_error("Failed to begin recording command buffer");
		}

		VkRenderPassBeginInfo renderBeginInfo { VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };

		_currentImageIndex = _swapchain.GetNextImageIndex(_imageAvailableSemaphore);

		renderBeginInfo.renderPass = _renderPass;
		renderBeginInfo.framebuffer = _swapchain.GetFrameBuffer(_currentImageIndex);
		renderBeginInfo.renderArea.offset = {0, 0};
		renderBeginInfo.renderArea.extent = _config.GetExtents();

		VkClearValue clearColor = {{{0, 0, 0, 1}}};
		renderBeginInfo.clearValueCount = 1;
		renderBeginInfo.pClearValues = &clearColor;

		vkCmdBeginRenderPass(_commandBuffer, &renderBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		vkCmdBindPipeline(_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _mainShader->GetPipeline());

		auto [width, height] = _config.GetExtents();

		VkViewport viewport { 
			0.0, 0.0f, 
			static_cast<float>(width), static_cast<float>(height), 
			0.0f, 1.0f};

		VkRect2D scissor { 
			0, 0,
			_config.GetExtents() };
	}

	void Graphics::PresentToScreen()
	{
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

		_assets->ReadBytes("shader.frag", output.code.get());

		return output;
	}

	void Graphics::WaitIdle()
	{
		vkDeviceWaitIdle(_device);
	}

	Graphics::~Graphics()
	{
		const VkAllocationCallbacks* const allocator = nullptr;

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