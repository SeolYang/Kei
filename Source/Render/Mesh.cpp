#include <PCH.h>
#include <Render/Mesh.h>
#include <Render/Vertex.h>

namespace sy::render
{
	Mesh::Mesh(const std::string_view name, const size_t numVertices, std::unique_ptr<vk::Buffer> vertexBuffer, const size_t numIndices,
		std::unique_ptr<vk::Buffer> indexBuffer) :
		NamedType(name),
		vertexBuffer(std::move(vertexBuffer)),
		indexBuffer(std::move(indexBuffer)),
		numVertices(numVertices),
		numIndices(numIndices)
	{
	}
}
