#include <PCH.h>
#include <Render/Mesh.h>
#include <VK/Buffer.h>

namespace sy::render
{
Mesh::~Mesh()
{
    /* empty */
}

Mesh::Mesh(const std::string_view name, const size_t numVertices, std::unique_ptr<vk::Buffer> vertexBuffer, const size_t numIndices, std::unique_ptr<vk::Buffer> indexBuffer, const Handle<Material> material) :
    NamedType(name),
    vertexBuffer(std::move(vertexBuffer)),
    indexBuffer(std::move(indexBuffer)),
    numVertices(numVertices),
    numIndices(numIndices),
    material(material)
{
}
} // namespace sy::render
