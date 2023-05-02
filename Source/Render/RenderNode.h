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

	[[nodiscard]] bool IsExecuteOnAsyncCompute() const { return bIsExecuteOnAsyncCompute; }
    void ExecuteOnAsyncCompute() { bIsExecuteOnAsyncCompute = true; }

	[[nodiscard]] auto GetSynchronizationIndex() const { return synchronizationIdx; }
    void SetSynchronizationIndex(const size_t idx) { synchronizationIdx = idx; }

	[[nodiscard]] auto GetDependencyLevel() const { return dependencyLevel; }
    void SetDependencyLevel(const size_t level) { dependencyLevel = level; }

private:
    void AsWriteDependency(std::string_view resourceName);
    void AsReadDependency(std::string_view resourceName, VkImageUsageFlags usage, vk::ETextureState state);
    void AsReadDependency(std::string_view resourceName, VkImageUsageFlags usage, vk::EBufferState state);

private:
    RenderGraph& renderGraph;
    const std::string name;
    bool bIsExecuteOnAsyncCompute = false;
    robin_hood::unordered_set<std::string> writeDependencies;
    robin_hood::unordered_set<std::string> readDependencies;
    size_t synchronizationIdx = 0;
    size_t dependencyLevel = 0;
};
} // namespace sy::render
