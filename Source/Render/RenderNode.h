#pragma once
#include <PCH.h>
#include <Render/RenderGraphResource.h>

namespace sy::render
{
class RenderGraph;
class RenderNode : public NonCopyable
{
public:
    RenderNode(RenderGraph& renderGraph, std::string_view name);
    ~RenderNode() override = default;

    RenderGraphTexture& CreateTexture(std::string_view textureName);
    RenderGraphBuffer& CreateBuffer(std::string_view bufferName);

    void AsGenaralSampledImage(std::string_view resourceName, std::span<const vk::TextureSubResource> subresources = {});

    RenderGraph& GetRenderGraph() { return renderGraph; }
    const RenderGraph& GetRenderGraph() const { return renderGraph; }
    std::string_view GetName() const { return name; }

    [[nodiscard]] bool HasAnyWriteDependency() const { return !writeDependencies.empty(); }
    [[nodiscard]] bool HasAnyReadDependency() const { return !readDependencies.empty(); }

	[[nodiscard]] const auto& GetWriteDependencies() const { return writeDependencies; }
    [[nodiscard]] const auto& GetReadDependencies() const { return readDependencies; }

private:
    void AsWriteDependency(std::string_view resourceName);
    void AsReadDependency(std::string_view resourceName, vk::ETextureState state);
    void AsReadDependency(std::string_view resourceName, vk::EBufferState state);

private:
    RenderGraph& renderGraph;
    const std::string name;
    robin_hood::unordered_set<std::string> writeDependencies;
    robin_hood::unordered_set<std::string> readDependencies;
};
} // namespace sy::render
