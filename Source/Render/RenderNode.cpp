#include <PCH.h>
#include <Render/RenderNode.h>
#include <Render/RenderGraph.h>

namespace sy::render
{
RenderNode::RenderNode(RenderGraph& renderGraph, const std::string_view name) :
    renderGraph(renderGraph),
    name(name)
{
}

RenderGraphTexture& RenderNode::CreateTexture(const std::string_view textureName)
{
    SY_ASSERT(!textureName.empty(), "Texture name is empty.");
    AsWriteDependency(textureName);
    RenderGraphTexture& texture = renderGraph.GetOrCreateTexture(textureName);
    texture.WriteBy(this->name);
    return texture;
}
RenderGraphBuffer& RenderNode::CreateBuffer(const std::string_view bufferName)
{
    SY_ASSERT(!bufferName.empty(), "Buffer name is empty.");
    AsWriteDependency(bufferName);
    RenderGraphBuffer& buffer = renderGraph.GetOrCreateBuffer(bufferName);
    buffer.WriteBy(this->name);
    return buffer;
}

void RenderNode::AsGenaralSampledImage(const std::string_view resourceName, const std::span<const vk::TextureSubResource> subresources)
{
    // TODO: extra care about subresources.
    AsReadDependency(resourceName, vk::ETextureState::AnyShaderReadSampledImage);
}

void RenderNode::AsReadDependency(const std::string_view resourceName, const vk::ETextureState state)
{
    SY_ASSERT(!resourceName.empty(), "Resource Name is empty.");
    auto& texture = renderGraph.GetOrCreateTexture(resourceName);
    readDependencies.insert(resourceName.data());
    texture.ReadBy(this->name, state);
}

void RenderNode::AsReadDependency(const std::string_view resourceName, const vk::EBufferState state)
{
    SY_ASSERT(!resourceName.empty(), "Resource Name is empty.");
    auto& buffer = renderGraph.GetOrCreateBuffer(resourceName);
    readDependencies.insert(resourceName.data());
    buffer.ReadBy(this->name, state);
}

void RenderNode::AsWriteDependency(const std::string_view resourceName)
{
    SY_ASSERT(!writeDependencies.contains(resourceName.data()), "Self write dependency occurs.");
    SY_ASSERT(!readDependencies.contains(resourceName.data()), "Self read-write dependency occurs.");
    writeDependencies.insert(resourceName.data());
}
} // namespace sy::render