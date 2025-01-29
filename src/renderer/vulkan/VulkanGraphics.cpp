#include "VulkanGraphics.hpp"

#include "Command.hpp"
#include "Config.hpp"
#include "Drawable.hpp"
#include "Sampler.hpp"
#include "Shader.hpp"
#include "SpritePacker.hpp"
#include "data/Assets.hpp"
#include "data/Common.hpp"
#include "data/Palette.hpp"
#include "data/Sprite.hpp"
#include "data/Tileset.hpp"
#include "device/Device.hpp"
#include "Window.hpp"
#include "device/Format.hpp"
#include "device/Queue.hpp"
#include "device/SwapChain.hpp"
#include "memory/BufferAllocator.hpp"
#include "memory/MemoryManager.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <glm/ext/matrix_transform.hpp>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <vector>

#include <vulkan/vulkan_core.h>

#include <glm/detail/qualifier.hpp>
#include <glm/vec2.hpp>

#include "DescriptorSetLayout.hpp"
	
	#include <diagnostic/Image.hpp>

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

		{
			auto fragmentShaderModule = _shaders.CreateShaderModule(ShaderStage::Fragment, ReadShaderCode("shaders/tex_frag.spv"));
			auto vertexShaderModule   = _shaders.CreateShaderModule(ShaderStage::Vertex, ReadShaderCode("shaders/tex_vert.spv"));
			array<uint32_t, 2> modules = { fragmentShaderModule, vertexShaderModule };

			_samplerLayout = DescriptorSetLayout::Builder(&_device)
				.AddBinding(BindSampler)
				.Create(allocator);

			_textureShaderIndex = _shaders.CreateShader(modules.data(), modules.size(), _swapchain.GetFormat(), &_samplerLayout);
		}

		{
			auto fragmentShaderModule = _shaders.CreateShaderModule(ShaderStage::Fragment, ReadShaderCode("shaders/pal_frag.spv"));
			auto vertexShaderModule   = _shaders.CreateShaderModule(ShaderStage::Vertex, ReadShaderCode("shaders/pal_vert.spv"));
			array<uint32_t, 2> modules = { fragmentShaderModule, vertexShaderModule };

			_standardLayout = DescriptorSetLayout::Builder(&_device)
				.AddBinding(BindSampler)
				.AddBinding(BindSampler)
				.Create(allocator);

			_mainShaderIndex = _shaders.CreateShader(modules.data(), modules.size(), _swapchain.GetFormat(), &_standardLayout);
		}

		CreateDescriptorPools();

		QueueFamilyIndices familyIndices = FindQueueFamilies(_device, _surface);
		_commandPool = CreateCommandPool(_device, familyIndices.graphicsFamily.value());
		_commandBuffer = CreateCommandBuffer(_device, _commandPool);

		CreateSyncObjects();

		_memoryManager = MemoryManager(&_device, allocator);

		_bufferAllocator = BufferAllocator(&_device, _device.GetGraphicsQueue(), _commandPool, 
																				&_memoryManager, allocator);
		_bufferAllocator.Initialize();

		_textureSampler = Sampler::Builder(_device)
			.AnisotropyEnabled(VK_FALSE)
			.MipLodBias(0.0f)
			.MipmapMode(VK_SAMPLER_MIPMAP_MODE_NEAREST)
			.Filtering(VK_FILTER_NEAREST)
			.RepeatMode(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE)
			.Create(allocator);

		_textureLinearInterpSampler = Sampler::Builder(_device)
			.AnisotropyEnabled(VK_TRUE)
			.Filtering(VK_FILTER_LINEAR)
			.RepeatMode(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE)
			.Create(allocator);	

		_tilesetImage = _bufferAllocator.CreateTextureImage(nullptr, 256, 1, 4);
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

	void Graphics::CreateDescriptorPools()
	{
		const VkAllocationCallbacks* allocator = nullptr;
		const uint32_t MAX_SETS = 200;

		array<VkDescriptorPoolSize, 1> poolSizes {
			VkDescriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_SETS),
		};

		VkDescriptorPoolCreateInfo poolInfo { VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };

		poolInfo.poolSizeCount = poolSizes.size();
		poolInfo.pPoolSizes    = poolSizes.data();
		poolInfo.maxSets       = MAX_SETS;
		poolInfo.flags         = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

		if (vkCreateDescriptorPool(_device, &poolInfo, allocator, &_descriptorPool) != VK_SUCCESS)
		{
			throw runtime_error("Failed to create descriptor pool");
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
		vector<data::SpriteData> spriteDataList;

		for(int i = 0; i < spriteSheetData.GetSpriteCount(); i++)
		{
			auto frameData = spriteSheetData.GetSpriteData(i);

			spriteDataList.push_back(frameData);
		}

		SpritePacker spritePacker(spriteDataList);

		auto atlas = spritePacker.CreateAtlas();
		auto atlasWidth = atlas.GetDimensions().x;
		auto atlasHeight = atlas.GetDimensions().y;

		int pixelSize = spriteSheetData.GetPixelSize();
		auto texturePixelData = std::make_shared<uint8_t[]>(atlasWidth * atlasHeight * pixelSize);

		for(int i = 0; i < spriteSheetData.GetSpriteCount(); i++)
		{
			data::SpriteRect frameRect = atlas.GetFrame(i);

			int  rowOffset = frameRect.y * atlasWidth * pixelSize;
			int  colOffset = frameRect.x * pixelSize;
			auto destinationData = texturePixelData.get() + rowOffset + colOffset;

			spriteSheetData.ReadPixelData(i, destinationData, atlasWidth * pixelSize);
		}

		auto image = _bufferAllocator.CreateTextureImage(texturePixelData.get(), atlasWidth, atlasHeight, pixelSize);
		auto spriteSheet = new SpriteSheet(spriteDataList, atlas, image);

		_drawables.push_back(spriteSheet);

		return spriteSheet;
	}

	DrawableHandle Graphics::LoadTileset(data::A_TilesetData& tilesetData, std::vector<bool>& usedTiles)
	{
		int usedTilesCount = std::count(usedTiles.begin(), usedTiles.end(), true);
		int tileSize       = tilesetData.GetTileSize();
		int tilesetSquare  = usedTilesCount * (tileSize * tileSize);
		int textureHeight  = pow(2, ceil(log2(tilesetSquare) * 0.5));

		bool halfWidth     = tilesetSquare <= textureHeight * textureHeight / 2;
		int  textureWidth   = halfWidth ? textureHeight / 2 : textureHeight;

		auto texturePixelData = std::make_shared<uint8_t[]>(textureWidth * textureHeight);

		int offsetX = 0, offsetY = 0;
		int index = 0;

		_tileMap.resize(tilesetData.GetTileCount(), 0);

		for (int i = 0; i < tilesetData.GetTileCount(); i++)
		{
			if (!usedTiles[i])
				continue;

			tilesetData.GetPixelData(i, texturePixelData.get(), offsetX + offsetY, textureWidth);

			offsetX += tileSize;

			if (offsetX >= textureWidth)
			{
				offsetX = 0;
				offsetY += textureWidth * tileSize;
			}

			_tileMap[i] = index++;
		}

		const int pixelSize = 1;

		auto image = _bufferAllocator.CreateTextureImage(texturePixelData.get(), textureWidth, textureHeight, pixelSize);
		Tileset *tileset = new Tileset(tilesetData, _tileMap, image, tileSize, textureWidth, textureHeight);

		_drawables.push_back(tileset);

		return tileset;
	}

	DrawableHandle Graphics::LoadImage(uint32_t* pixels, uint32_t width, uint32_t height)
	{
		int textureWidth  = pow(2, ceil(log2(width)));
		int textureHeight = pow(2, ceil(log2(height)));

		const int pixelSize = 4;

		auto texturePixels = std::make_shared<uint32_t[]>(textureWidth * textureHeight);

		data::CopyImage(pixels, width, texturePixels.get(), { 0, 0, width, height }, textureWidth);

		auto image = _bufferAllocator.CreateTextureImage(texturePixels.get(), textureWidth, textureHeight, pixelSize);
		Picture *picture = new Picture(width, height, textureWidth, textureHeight, image);

		_drawables.push_back(picture);

		return picture;
	}

	DrawCall* Graphics::UseDrawCall(DrawableHandle drawableHandle)
	{
		// Break into another draw call
		if (_currentDrawCall == nullptr || drawableHandle != _currentDrawCall->drawable)
		{
			// Validate new drawable
			auto drawable = ConvertToDrawable(drawableHandle);

			_drawCalls.emplace_back(drawable);
			return &_drawCalls.back();
		}

		return _currentDrawCall;
	}

	void Graphics::Draw(DrawableHandle drawableHandle, frameIndex frame, data::position position)
	{
		_currentDrawCall = UseDrawCall(drawableHandle);

		array<Vertex, 10> polygonVertices;

		auto count = _currentDrawCall->drawable->GetPolygon(frame, polygonVertices, polygonVertices.size());

		_currentDrawCall->vertexCount += count;

		if (count > polygonVertices.size())
		{
			throw runtime_error("Too much polygons");
		}

		double reverseWidth  = 1.0 / _config.GetExtents().width;
		double reverseHeight = 1.0 / _config.GetExtents().height;

		for(int i = 0; i < count; i++)
		{
			auto& vertex = polygonVertices[i];

			vertex.pos   += position - _currentPosition;
			vertex.pos.x *= reverseWidth;
			vertex.pos.y *= reverseHeight;

			vertex.pos = vertex.pos * 2.0f - 1.0f;
		}

		_bufferAllocator.WriteToStreamBuffer(_currentDrawCall->streamData, sizeof(Vertex) * count, polygonVertices.data());

		_drawablesCache[_drawablesCacheIndex] = _currentDrawCall->drawable;
		_drawablesCacheIndex = (_drawablesCacheIndex + 1) % _drawablesCache.size();
	}

	void Graphics::Draw(DrawableHandle drawableHandle, data::position pos, uint32_t width, uint32_t height)
	{
		_currentDrawCall = UseDrawCall(drawableHandle);

		array<Vertex, 10> polygonVertices;

		auto count = _currentDrawCall->drawable->GetPolygon(0, polygonVertices, polygonVertices.size(), width, height);

		_currentDrawCall->vertexCount += count;

		if (count > polygonVertices.size())
		{
			throw runtime_error("Too much polygons");
		}

		double reverseWidth  = 1.0 / _config.GetExtents().width;
		double reverseHeight = 1.0 / _config.GetExtents().height;

		for(int i = 0; i < count; i++)
		{
			auto& vertex = polygonVertices[i];

			vertex.pos   += pos - _currentPosition;
			vertex.pos.x *= reverseWidth;
			vertex.pos.y *= reverseHeight;

			vertex.pos = vertex.pos * 2.0f - 1.0f;
		}

		_bufferAllocator.WriteToStreamBuffer(_currentDrawCall->streamData, sizeof(Vertex) * count, polygonVertices.data());

		_drawablesCache[_drawablesCacheIndex] = _currentDrawCall->drawable;
		_drawablesCacheIndex = (_drawablesCacheIndex + 1) % _drawablesCache.size();
	}

	void Graphics::FreeDrawable(DrawableHandle drawableHandle)
	{
		// TODO: remove
		if (drawableHandle == nullptr)
		{
			return;
		}

		auto iterator = FindDrawable(drawableHandle);
		auto drawable = *iterator;

		_bufferAllocator.FreeImage(drawable->GetImage());

		_drawables.erase(iterator);

		// Remove drawable from cache
		for(int i = 0; i < _drawablesCache.size(); i++)
		{
			if (_drawablesCache[i] == drawable)
			{
				_drawablesCache[i] = nullptr;
				break;
			}
		}

		switch (drawable->GetType()) {
			case SpriteSheetType:
				delete dynamic_cast<SpriteSheet*>(drawable);
				break;
			
			case TilesetType:
				delete dynamic_cast<Tileset*>(drawable);
				break;

			case PictureType:
				delete dynamic_cast<Picture*>(drawable);
				break;
		}
	}

	void Graphics::SetTilesetPalette(data::Palette& palette)
	{
		auto data = reinterpret_cast<const uint8_t*>(palette.GetColors());

		_bufferAllocator.UpdateImageData(_tilesetImage, data, 256, 1, 4);
	}

	void Graphics::SetView(data::position pos)
	{
		_currentPosition = pos;
	}
	
	void Graphics::ClearDescriptorPool()
	{
		if (_descriptorSetCount == 0)
			return;

		if (vkFreeDescriptorSets(_device, _descriptorPool, _descriptorSetCount, _descriptorSets) != VK_SUCCESS)
		{
			throw runtime_error("Failed to free descriptor sets");
		}

		_descriptorSetCount = 0;
	}

	void Graphics::BeginRendering()
	{
		// Wait for previous frame to be rendered
		vkWaitForFences(_device, 1, &_fence, VK_TRUE, UINT64_MAX);
		vkResetFences(_device, 1, &_fence);

		vkResetCommandBuffer(_commandBuffer, 0);

		_bufferAllocator.OnBeginRendering();

		_drawCalls.clear();
		_currentDrawCall = nullptr;

		ClearDescriptorPool();
	}

	void Graphics::AllocateDescriptorSets()
	{
		if (_drawCalls.size() == 0)
		{
			return;
		}

		// for each drawable allocate descriptor set
		vector<VkDescriptorSetLayout> layouts;

		layouts.reserve(_drawCalls.size());

		for(int i = 0; i < _drawCalls.size(); i++)
		{
			auto& drawCall = _drawCalls[i];
			
			if (drawCall.drawable->GetType() == PictureType)
			{
				layouts.push_back(_samplerLayout);
			}
			else
			{
				layouts.push_back(_standardLayout);
			}
		}

		_descriptorSetCount = layouts.size();

		VkDescriptorSetAllocateInfo   allocInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
		allocInfo.descriptorPool     = _descriptorPool;
		allocInfo.pSetLayouts        = layouts.data();
		allocInfo.descriptorSetCount = layouts.size();

		if (vkAllocateDescriptorSets(_device, &allocInfo, _descriptorSets) != VK_SUCCESS)
		{
			throw runtime_error("Failed to create descriptor pool");
		}
	}

	void Graphics::WriteDescriptorSets()
	{
		for(int i = 0; i < _drawCalls.size(); i++)
		{
			auto& drawCall = _drawCalls[i];

			if (drawCall.drawable->GetType() == PictureType)
			{
				array<VkWriteDescriptorSet, 1> descriptorWrites{};

				VkDescriptorImageInfo textureInfo {
					.sampler = _textureLinearInterpSampler,
					.imageView = drawCall.drawable->GetImageView(),
					.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
				};

				descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrites[0].dstSet = _descriptorSets[i];
				descriptorWrites[0].dstBinding = 0;
				descriptorWrites[0].dstArrayElement = 0;
				descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				descriptorWrites[0].descriptorCount = 1;
				descriptorWrites[0].pImageInfo = &textureInfo;

				vkUpdateDescriptorSets(_device, descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);
			}
			else
			{
				array<VkWriteDescriptorSet, 2> descriptorWrites{};

				VkDescriptorImageInfo textureInfo {
					.sampler = _textureSampler,
					.imageView = drawCall.drawable->GetImageView(),
					.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
				};

				descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrites[0].dstSet = _descriptorSets[i];
				descriptorWrites[0].dstBinding = 0;
				descriptorWrites[0].dstArrayElement = 0;
				descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				descriptorWrites[0].descriptorCount = 1;
				descriptorWrites[0].pImageInfo = &textureInfo;

				VkDescriptorImageInfo paletteInfo {
					.sampler = _textureSampler,
					.imageView = _tilesetImage->GetViewHandle(),
					.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
				};

				descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrites[1].dstSet = _descriptorSets[i];
				descriptorWrites[1].dstBinding = 1;
				descriptorWrites[1].dstArrayElement = 0;
				descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				descriptorWrites[1].descriptorCount = 1;
				descriptorWrites[1].pImageInfo = &paletteInfo;

				vkUpdateDescriptorSets(_device, descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);
			}
		}
	}

	void Graphics::PresentToScreen()
	{
		_bufferAllocator.OnPrepareForPresentation();

		AllocateDescriptorSets();
		WriteDescriptorSets();

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

		VkClearValue clearColor = {{{0, 0, 0, 1}}};
		renderBeginInfo.clearValueCount = 1;
		renderBeginInfo.pClearValues = &clearColor;

		vkCmdBeginRenderPass(_commandBuffer, &renderBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		// Prepare viewport

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
		//   Draw

		for(int i = 0; i < _drawCalls.size(); i++)
		{
			auto& drawCall = _drawCalls[i];

			VkPipeline pipeline;
			VkPipelineLayout pipelineLayout;

			if (drawCall.drawable->GetType() == PictureType)
			{
				pipeline = _shaders.GetShaderPipeline(_textureShaderIndex);
				pipelineLayout = _shaders.GetShaderPipelineLayout(_textureShaderIndex);
			}
			else
			{
				pipeline = _shaders.GetShaderPipeline(_mainShaderIndex);
				pipelineLayout = _shaders.GetShaderPipelineLayout(_mainShaderIndex);
			}

			drawCall.streamData.BindToCommandBuffer(_commandBuffer);

			vkCmdBindPipeline(_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
			vkCmdBindDescriptorSets(_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, 
															pipelineLayout, 0, 1, &_descriptorSets[i], 0, VK_NULL_HANDLE);
			vkCmdDraw(_commandBuffer, drawCall.vertexCount, 1, 0, 0);
		}

		// ==============================================

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
		presentInfo.pSwapchains    = swapchains.data();
		presentInfo.pImageIndices  = imageIndices.data();

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

	A_VulkanDrawable* Graphics::ConvertToDrawable(DrawableHandle handle)
	{
		// Validate new drawable
		auto vulkanDrawable = reinterpret_cast<A_VulkanDrawable*>(handle);
		bool isCached = std::find(_drawablesCache.begin(), _drawablesCache.end(), vulkanDrawable) != _drawablesCache.end();

		if (isCached)
		{
			return vulkanDrawable;
		}

		// If it's not in the cache check in the list
		return *FindDrawable(handle);
	}

	std::vector<A_VulkanDrawable*>::iterator Graphics::FindDrawable(DrawableHandle handle)
	{
		// Validate new drawable
		auto vulkanDrawable = reinterpret_cast<A_VulkanDrawable*>(handle);

		// Check if drawable in cache
		auto it = std::find(_drawables.begin(), _drawables.end(), vulkanDrawable);
		if (it == _drawables.end())
		{
			throw runtime_error("Incompatible drawable object");
		}

		return it;
	}

	void Graphics::Release()
	{
		const VkAllocationCallbacks* const allocator = nullptr;

		if (_descriptorPool != VK_NULL_HANDLE)
		{
			vkDestroyDescriptorPool(_device, _descriptorPool, allocator);
		}

		_descriptorPool = VK_NULL_HANDLE;

		_standardLayout.Destroy();

		_samplerLayout.Destroy();

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

				case PictureType:
					delete dynamic_cast<Picture*>(drawable);
					break;
			}
		}

		_textureSampler.Destroy();

		_textureLinearInterpSampler.Destroy();

		_bufferAllocator.Release();

		_memoryManager.Release();

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
}