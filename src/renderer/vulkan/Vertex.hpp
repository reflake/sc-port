#pragma once

#include <array>
#include <glm/ext/vector_float2.hpp>
#include <vulkan/vulkan_core.h>

namespace renderer::vulkan
{
	struct Vertex
	{
		glm::vec2 pos = { 0, 0 };
		glm::vec2 texCoord = { 0, 0 };

		static VkVertexInputBindingDescription GetBindingDescription();

		static std::array<VkVertexInputAttributeDescription, 2> GetAttributeDescriptions();
	};
}