#pragma once
#include <PCH.h>
#include <Render/RenderGraphResource.h>

namespace sy::render
{
class RenderNode;
// #todo How about simultaneous of frame? multiple instance?
// ex.. reesource.Instantiate(frameIndex)...
// resource::array<Resource, 2> instances;
class RenderGraph : public NonCopyable
{
private:
    constexpr static size_t NumOfSupportedQueues = 2; // Graphics, Async Compute
    constexpr static vk::EQueueType MostCompetentQueue = vk::EQueueType::Graphics;

public:
    RenderGraph(vk::VulkanContext& vulkanContext);
    ~RenderGraph();

    RenderGraphTexture& GetOrCreateTexture(std::string_view name);
    RenderGraphBuffer& GetOrCreateBuffer(std::string_view name);

    [[nodiscard]] bool ContainsResource(const std::string_view name) const { return textureMap.contains(name.data()) || bufferMap.contains(name.data()); }

    template <typename T>
    void AppendNode(std::unique_ptr<T> node)
    {
        nodes.emplace_back(std::unique_ptr<RenderNode>(static_cast<RenderNode*>(node.release())));
    }

    template <typename T, typename... Args>
    RenderNode& EmplaceNode(Args&&... args)
    {
        AppendNode(std::make_unique<T>(std::forward<Args>(args)...));
        return *nodes.back();
    }

    void Compile();

private:
    static size_t QueryQueueIndex(vk::EQueueType queueType);

    void TopologicalSort();
    void DFS(size_t nodeIdx, const robin_hood::unordered_map<std::string, size_t> nodeIndexMap, std::vector<size_t>& sorted, std::vector<bool>& visited, std::vector<bool>& onStack);

    // SSIS: Sufficient Synchronization Index Set
    void InitSSIS();
    void BuildNodeSyncrhonizationIndexMap(std::array<std::vector<std::string_view>, NumOfSupportedQueues> groupedNodesByQueue);
    std::array<std::vector<std::string_view>, NumOfSupportedQueues> GroupNodesByQueue();
    void ResetSSIS();
    void BuildSSIS();

private:
    vk::VulkanContext& vulkanContext;
    std::vector<std::unique_ptr<RenderNode>> nodes;
    robin_hood::unordered_map<std::string, std::unique_ptr<RenderGraphTexture>> textureMap = {};
    robin_hood::unordered_map<std::string, std::unique_ptr<RenderGraphBuffer>> bufferMap = {};
    robin_hood::unordered_map<std::string, size_t> nodeSyncIndexMap;
};
} // namespace sy::render