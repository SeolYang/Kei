#include <PCH.h>
#include <Render/RenderGraph.h>
#include <Render/RenderNode.h>
#include <VK/Texture.h>
#include <VK/Buffer.h>

namespace sy::render
{
// #todo clean up and simplify all things!!
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

size_t RenderGraph::QueryQueueIndex(const RenderNode& node)
{
    return QueryQueueIndex(node.IsExecuteOnAsyncCompute() ? vk::EQueueType::Compute : MostCompetentQueue);
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
            DFS(0, idx, nodeIndexMap, sorted, visited, onStack);
        }
    }

    std::reverse(sorted.begin(), sorted.end());
    nodes = Permute(std::move(nodes), sorted);
}

void RenderGraph::DFS(const size_t depth, const size_t nodeIdx, const robin_hood::unordered_map<std::string, size_t> nodeIndexMap, std::vector<size_t>& sorted, std::vector<bool>& visited, std::vector<bool>& onStack)
{
    SY_ASSERT(!onStack[nodeIdx], "Found circular dependency in graph.");
    if (!visited[nodeIdx])
    {
        robin_hood::unordered_set<size_t> readByDependencies;
        for (const auto& writeResourceName : nodes[nodeIdx]->GetWriteDependencies())
        {
            robin_hood::unordered_set<std::string> readersOfResource;
            if (textureMap.contains(writeResourceName))
            {
                readersOfResource = textureMap[writeResourceName]->GetReaders();
            }
            else if (bufferMap.contains(writeResourceName))
            {
                readersOfResource = bufferMap[writeResourceName]->GetReaders();
            }
            for (const auto& dependentNodeName : textureMap[writeResourceName]->GetReaders())
            {
                const auto idx = GetNodeIndex(dependentNodeName);
                SY_ASSERT(idx.has_value(), "Dependent Node name is not valid.");
                readByDependencies.insert(*idx);
            }
        }

        visited[nodeIdx] = true;
        onStack[nodeIdx] = true;
        for (const size_t readNode : readByDependencies)
        {
            DFS(depth + 1, readNode, nodeIndexMap, sorted, visited, onStack);
        }
        onStack[nodeIdx] = false;
        nodes[nodeIdx]->SetDependencyLevel(depth);
        sorted.emplace_back(nodeIdx);
    }
}

void RenderGraph::Compile()
{
    TopologicalSort();
    InitSSIS();
    BuildMinDependencyLevelSyncPoints();
    // #todo Schedule synchronization / resource state transitions
    // if 2 or more read dependencies exist at same dependency level.
    // Should i force promote state to AnyXXX state?
	for (size_t dl = 0; dl <= nodes.back()->GetDependencyLevel(); ++dl)
	{
		if (!minDependencyLevelSyncPoints[dl].empty())
		{
            spdlog::info("Dependency Level {}, required to sync with below queues.", dl);
            for (const size_t queueIdx : minDependencyLevelSyncPoints[dl])
            {
                spdlog::info("Q{}", queueIdx);
            }
		}
	}
}

void RenderGraph::InitSSIS()
{
    GroupNodesByQueue();
    BuildNodeSyncrhonizationIndexMap();
    ResetSSIS();
    BuildSSIS();
}

void RenderGraph::BuildNodeSyncrhonizationIndexMap()
{
    size_t idxOffset = 0;
    for (const auto& groupedNodes : groupedNodesByQueue)
    {
        for (size_t idx = 0; idx < groupedNodes.size(); ++idx)
        {
            nodes[groupedNodes[idx]]->SetSynchronizationIndex(idxOffset + idx + 1);
        }
        idxOffset += groupedNodes.size();
    }
}

void RenderGraph::GroupNodesByQueue()
{
    for (size_t idx = 0; idx < nodes.size(); ++idx)
    {
        const auto& node = nodes[idx];
        groupedNodesByQueue[QueryQueueIndex(*node)].push_back(idx);
    }
}

void RenderGraph::ResetSSIS()
{
    ssises.resize(nodes.size());
    std::fill(ssises.begin(), ssises.end(), SSIS{});
}

void RenderGraph::BuildSSIS()
{
    for (const auto& nodePtr : nodes)
    {
        const auto& node = *nodePtr;
        const size_t syncIdx = node.GetSynchronizationIndex();
        auto& ssis = GetSSIS(syncIdx);
        if (node.HasAnyReadDependency())
        {
            SY_ASSERT(syncIdx != 0, "Synchronization Index == 0 only for read-independent node.");
            ssis.CopyOtherNextToNow(GetSSIS(syncIdx - 1));

            const auto& resourceReadDependencies = node.GetReadDependencies();
            for (const std::string_view resourceName : resourceReadDependencies)
            {
                const auto writerNodeNameOpt = QueryWriterFromResource(resourceName);
                SY_ASSERT(writerNodeNameOpt.has_value(), "Resource Name does not has any valid writer.");
                auto writerNodeIdxOpt = GetNodeIndex(*writerNodeNameOpt);
                SY_ASSERT(writerNodeIdxOpt.has_value(), "Invalid writer node name.");
                auto& writerNode = *nodes[*writerNodeIdxOpt];

                const size_t writerNodeQueueIdx = QueryQueueIndex(writerNode);
                if (writerNode.IsExecuteOnAsyncCompute() != node.IsExecuteOnAsyncCompute())
                {
                    auto& writerNodeSSIS = GetSSIS(*writerNodeIdxOpt);
                    ssis.UpdateNow(writerNodeQueueIdx, GetSSIS(writerNode.GetSynchronizationIndex()));
                }
            }
        }

        ssis.UpdateNext(QueryQueueIndex(node), syncIdx);
    }
}

RefOptional<RenderNode> RenderGraph::GetNode(const std::string_view name)
{
    auto idxOpt = GetNodeIndex(name);
    if (idxOpt)
    {
        return *nodes[*idxOpt];
    }

    return std::nullopt;
}

std::optional<std::string_view> RenderGraph::QueryWriterFromResource(const std::string_view resourceName) const
{
    if (textureMap.contains(resourceName.data()))
    {
        return textureMap.at(resourceName.data())->GetWriter();
    }
    else if (bufferMap.contains(resourceName.data()))
    {
        return bufferMap.at(resourceName.data())->GetWriter();
    }

    return std::nullopt;
}

RenderGraph::SSIS& RenderGraph::GetSSIS(const size_t synchronizationIdx)
{
    return ssises[synchronizationIdx - 1];
}

std::optional<size_t> RenderGraph::GetNodeIndex(const std::string_view name) const
{
    for (size_t idx = 0; idx < nodes.size(); ++idx)
    {
        const auto& node = *nodes[idx];
        if (node.GetName() == name)
        {
            return idx;
        }
    }

    return std::nullopt;
}

void RenderGraph::BuildMinDependencyLevelSyncPoints()
{
    const size_t maxDependencyLevel = nodes.back()->GetDependencyLevel();
    minDependencyLevelSyncPoints.resize(maxDependencyLevel + 1);
	for (size_t dl = 0; dl <= maxDependencyLevel; ++dl)
	{
		for (size_t queueIdx = 0; queueIdx < NumOfSupportedQueues; ++queueIdx)
		{
            const auto& groupedNodesIdx = groupedNodesByQueue[queueIdx];
            bool bFoundSynchronizationPoint = false;
			for (const size_t nodeIdx : groupedNodesIdx)
			{
				if (bFoundSynchronizationPoint)
				{
                    break;
				}
                else if (nodes[nodeIdx]->HasAnyReadDependency() && (nodes[nodeIdx]->GetDependencyLevel() == dl))
				{
                    for (size_t otherQueueIdx = 0; otherQueueIdx < NumOfSupportedQueues; ++otherQueueIdx)
                    {
                        if (queueIdx != otherQueueIdx && !minDependencyLevelSyncPoints[dl].contains(otherQueueIdx))
                        {
                            const size_t syncIdx = nodes[nodeIdx]->GetSynchronizationIndex();
                            const auto& ssis = GetSSIS(syncIdx);
                            const auto& prevSSIS = GetSSIS(syncIdx - 1);

							if (ssis.Now[otherQueueIdx] != prevSSIS.Next[otherQueueIdx])
							{
                                minDependencyLevelSyncPoints[dl].insert(otherQueueIdx);
								bFoundSynchronizationPoint = true;
                                break;
							}
                        }
                    }
				}
			}
		}
	}
}
} // namespace sy::render