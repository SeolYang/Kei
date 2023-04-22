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
}

void RenderGraph::DFS(const size_t nodeIdx, const robin_hood::unordered_map<std::string, size_t> nodeIndexMap, std::vector<size_t>& sorted, std::vector<bool>& visited, std::vector<bool>& onStack)
{
    SY_ASSERT(!onStack[nodeIdx], "Found circular dependency in graph.");
	if (!visited[nodeIdx])
	{
        robin_hood::unordered_set<size_t> readByDependencies;
        for (const auto& writeResourceName : nodes[nodeIdx]->GetWriteDependencies())
        {
            if (textureMap.contains(writeResourceName.data()))
            {
                for (const auto& dependentNodeName : textureMap[writeResourceName.data()]->GetReaders())
                {
                    readByDependencies.insert(nodeIndexMap.at(dependentNodeName));
                }
            }
            else if (bufferMap.contains(writeResourceName.data()))
            {
                for (const auto& dependentNodeName : bufferMap[writeResourceName.data()]->GetReaders())
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
	// Calculate Synchronization
}

} // namespace sy::render