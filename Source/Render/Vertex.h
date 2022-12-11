#pragma once
#include <Core/Core.h>
#include <VK/VertexInputBuilder.h>

namespace sy::render
{
	enum class EVertexType
	{
		VertexPT,
		VertexPTN,
		/** @todo Add more advanced types of vertex. */
	};

	template <typename VertexType>
	vk::VertexInputBuilder BuildVertexInputLayout();

	struct VertexPT
	{
		glm::vec3 Position;
		glm::vec2 TexCoords;
	};

	template <>
	inline vk::VertexInputBuilder BuildVertexInputLayout<VertexPT>()
	{
		using Vertex = VertexPT;
		vk::VertexInputBuilder builder;
		builder.AddVertexInputBinding<Vertex>(0, VK_VERTEX_INPUT_RATE_VERTEX)
			.AddVertexInputAttribute(0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, Position))
			.AddVertexInputAttribute(1, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, TexCoords));
		return builder;
	}

	struct VertexPTN
	{
		glm::vec3 Position;
		glm::vec2 TexCoords;
		glm::vec3 Normal;
	};

	template <>
	inline vk::VertexInputBuilder BuildVertexInputLayout<VertexPTN>()
	{
		using Vertex = VertexPTN;
		vk::VertexInputBuilder builder;
		builder.AddVertexInputBinding<Vertex>(0, VK_VERTEX_INPUT_RATE_VERTEX)
			.AddVertexInputAttribute(0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, Position))
			.AddVertexInputAttribute(1, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, TexCoords))
			.AddVertexInputAttribute(2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, Normal));
		return builder;
	}

	inline vk::VertexInputBuilder BuildVertexInputLayout(const EVertexType type)
	{
		switch (type)
		{
		case EVertexType::VertexPT:
			return BuildVertexInputLayout<VertexPT>();

		case EVertexType::VertexPTN:
			return BuildVertexInputLayout<VertexPTN>();

		default:
			return {};
		}
	}
}