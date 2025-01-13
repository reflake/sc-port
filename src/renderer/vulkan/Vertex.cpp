#include "Vertex.hpp"
#include <vulkan/vulkan_core.h>

namespace renderer::vulkan
{
	using std::array;

	using BindingDescription = VkVertexInputBindingDescription;

	BindingDescription Vertex::GetBindingDescription()
	{
		const BindingDescription bindingDescription {
			.binding = 0,
			.stride = sizeof(Vertex),
			.inputRate = VK_VERTEX_INPUT_RATE_VERTEX
		};

		return bindingDescription;
	}

	using AttributeDescription = VkVertexInputAttributeDescription;

	array<AttributeDescription, 2> Vertex::GetAttributeDescriptions()
	{
		array<AttributeDescription, 2> array = {
			AttributeDescription { 0, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, pos) },
			AttributeDescription { 1, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, texCoord) }
		};

		return array;
	}
}