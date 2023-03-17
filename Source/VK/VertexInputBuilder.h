#pragma once
#include <PCH.h>

namespace sy::vk
{
	class VertexInputBuilder
	{
	public:
		VertexInputBuilder& Clear();

		template <typename T>
		VertexInputBuilder& AddVertexInputBinding(uint32_t binding,
			const VkVertexInputRate inputRate)
		{
			vertexInputBindingDescriptions.emplace_back(
				binding, static_cast<uint32_t>(sizeof(T)), inputRate);
			return *this;
		}

		VertexInputBuilder& AddVertexInputAttribute(uint32_t location,
			uint32_t binding,
			VkFormat format,
			uint32_t offset);

		VkPipelineVertexInputStateCreateInfo Build() const;

	private:
		std::vector<VkVertexInputBindingDescription> vertexInputBindingDescriptions;
		std::vector<VkVertexInputAttributeDescription> vertexInputAttributeDescriptions;
	};
} // namespace sy::vk
