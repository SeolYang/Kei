#pragma once
#include <PCH.h>
#include <VK/BufferBuilder.h>

namespace sy::vk
{
	class Buffer;
	class VulkanContext;
}

namespace sy::render
{
	class Material;

	class Mesh : public NamedType
	{
	public:
		template <typename VertexType, typename IndexType = render::IndexType>
		static std::unique_ptr<Mesh> Create(const std::string_view name, const vk::VulkanContext& vulkanContext,
		                                    const std::span<const VertexType> vertices,
		                                    const std::span<const IndexType> indices)
		{
			const vk::BufferBuilder vertexBufferBuilder =
					vk::BufferBuilder::VertexBufferTemplate(vulkanContext)
					.SetName(std::format("{}_Vertex_Buffer", name))
					.SetDataToTransferWithSize(vertices);

			const vk::BufferBuilder indexBufferBuilder =
					vk::BufferBuilder::IndexBufferTemplate(vulkanContext)
					.SetName(std::format("{}_Index_Buffer", name))
					.SetDataToTransferWithSize(indices);

			return std::unique_ptr<Mesh>(new Mesh(name,
			                                      vertices.size(), vertexBufferBuilder.Build(),
			                                      indices.size(), indexBufferBuilder.Build()));
		}

		[[nodiscard]] const vk::Buffer& GetVertexBuffer() const
		{
			return *vertexBuffer;
		}

		[[nodiscard]] const vk::Buffer& GetIndexBuffer() const
		{
			return *indexBuffer;
		}

		[[nodiscard]] size_t GetNumVertices() const
		{
			return numVertices;
		}

		[[nodiscard]] size_t GetNumIndices() const
		{
			return numIndices;
		}

		~Mesh() override;

	private:
		Mesh(std::string_view name, size_t numVertices, std::unique_ptr<vk::Buffer> vertexBuffer, size_t numIndices,
		     std::unique_ptr<vk::Buffer> indexBuffer);

	private:
		std::unique_ptr<vk::Buffer> vertexBuffer;
		std::unique_ptr<vk::Buffer> indexBuffer;

		const size_t numVertices;
		const size_t numIndices;
	};
}
