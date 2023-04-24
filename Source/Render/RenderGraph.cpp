#include <PCH.h>
#include <Render/RenderGraph.h>
#include <Render/RenderNode.h>
#include <VK/Texture.h>
#include <VK/Buffer.h>

namespace sy::render
{
RenderGraph::RenderGraph(vk::VulkanContext& vulkanContext) :
    vulkanContext(vulkanContext)
{
}

RenderGraph::~RenderGraph()
{
}

RenderGraphTexture& RenderGraph::GetOrCreateTexture(const std::string_view name)
{
    if (!textureMap.contains(name.data()))
    {
        textureMap[name.data()] = std::make_unique<RenderGraphTexture>(vulkanContext, name);
    }

    return *textureMap.at(name.data());
}

RenderGraphBuffer& RenderGraph::GetOrCreateBuffer(std::string_view name)
{
    if (!bufferMap.contains(name.data()))
    {
        bufferMap[name.data()] = std::make_unique<RenderGraphBuffer>(vulkanContext, name);
    }

    return *bufferMap.at(name.data());
}

size_t RenderGraph::QueryQueueIndex(const vk::EQueueType queueType)
{
    switch (queueType)
    {
        case vk::EQueueType::Graphics:
            return 0;

        case vk::EQueueType::Compute:
            return 1;

        default:
            break;
    }

    return 0;
}

void RenderGraph::TopologicalSort()
{
    robin_hood::unordered_map<std::string, size_t> nodeIndexMap;
    for (size_t idx = 0; idx < nodes.size(); ++idx)
    {
        nodeIndexMap[nodes[idx]->GetName().data()] = idx;
    }

    std::vector<size_t> sorted;
    sorted.reserve(nodes.size());
    std::vector<bool> visited(nodes.size());
    std::vector<bool> onStack(nodes.size());

    for (size_t idx = 0; idx < nodes.size(); ++idx)
    {
        if (!nodes[idx]->HasAnyReadDependency() && nodes[idx]->HasAnyWriteDependency())
        {
            DFS(idx, nodeIndexMap, sorted, visited, onStack);
        }
    }

    std::reverse(sorted.begin(), sorted.end());
    nodes = Permute(std::move(nodes), sorted);
}

void RenderGraph::DFS(const size_t nodeIdx, const robin_hood::unordered_map<std::string, size_t> nodeIndexMap, std::vector<size_t>& sorted, std::vector<bool>& visited, std::vector<bool>& onStack)
{
    SY_ASSERT(!onStack[nodeIdx], "Found circular dependency in graph.");
    if (!visited[nodeIdx])
    {
        robin_hood::unordered_set<size_t> readByDependencies;
        for (const auto& writeResourceName : nodes[nodeIdx]->GetWriteDependencies())
        {
            if (textureMap.contains(writeResourceName))
            {
                for (const auto& dependentNodeName : textureMap[writeResourceName]->GetReaders())
                {
                    readByDependencies.insert(nodeIndexMap.at(dependentNodeName));
                }
            }
            else if (bufferMap.contains(writeResourceName))
            {
                for (const auto& dependentNodeName : bufferMap[writeResourceName]->GetReaders())
                {
                    readByDependencies.insert(nodeIndexMap.at(dependentNodeName));
                }
            }
        }

        visited[nodeIdx] = true;
        onStack[nodeIdx] = true;
        for (const size_t readNode : readByDependencies)
        {
            DFS(readNode, nodeIndexMap, sorted, visited, onStack);
        }
        onStack[nodeIdx] = false;
        sorted.emplace_back(nodeIdx);
    }
}

void RenderGraph::Compile()
{
    TopologicalSort();
    InitSSIS();

	spdlog::info("Topologically sorted Nodes: ");
	for (const auto& node : nodes)
	{
        spdlog::info("Name: {}, Synchronization Index: {}", node->GetName(), nodeSyncIndexMap[node->GetName().data()]);
	}
}

void RenderGraph::InitSSIS()
{
    BuildNodeSyncrhonizationIndexMap(GroupNodesByQueue());
    ResetSSIS();
    BuildSSIS();
}

void RenderGraph::BuildNodeSyncrhonizationIndexMap(const std::array<std::vector<std::string_view>, NumOfSupportedQueues> groupedNodesByQueue)
{
    size_t idxOffset = 0;
	for (const auto& groupedNodes : groupedNodesByQueue)
	{
		for (size_t idx = 0; idx < groupedNodes.size(); ++idx)
		{
            nodeSyncIndexMap[groupedNodes[idx].data()] = idxOffset + idx + 1;
		}
        idxOffset += groupedNodes.size();
	}
}

std::array<std::vector<std::string_view>, RenderGraph::NumOfSupportedQueues> RenderGraph::GroupNodesByQueue()
{
    std::array<std::vector<std::string_view>, RenderGraph::NumOfSupportedQueues> groupedNodes;
    for (const auto& node : nodes)
    {
        if (node->IsExecuteOnAsyncCompute())
        {
            groupedNodes[QueryQueueIndex(vk::EQueueType::Compute)].push_back(node->GetName());
        }
        else
        {
            groupedNodes[QueryQueueIndex(MostCompetentQueue)].push_back(node->GetName());
        }
    }

    return groupedNodes;
}

void RenderGraph::ResetSSIS()
{
}

void RenderGraph::BuildSSIS()
{
}
} // namespace sy::render