#include <PCH.h>
#include <VK/CommandPoolAllocator.h>
#include <VK/FrameTracker.h>
#include <VK/VulkanRHI.h>
#include <VK/CommandPool.h>

namespace sy::vk
{
CommandPoolAllocator::CommandPoolAllocator(VulkanContext& vulkanContext, const FrameTracker& frameTracker) :
    vulkanContext(vulkanContext), frameTracker(frameTracker)
{
}

CommandPoolAllocator::~CommandPoolAllocator()
{
}

void CommandPoolAllocator::Startup()
{
}

void CommandPoolAllocator::Shutdown()
{
    for (auto& cmdPoolVec : cmdPools)
    {
        cmdPoolVec.clear();
    }
}

CommandPool& CommandPoolAllocator::RequestCommandPool(const EQueueType queueType)
{
    thread_local robin_hood::unordered_map<EQueueType, std::array<CommandPool*, NumMaxInFlightFrames>>
        localCmdPools;
    if (!localCmdPools.contains(queueType))
    {
        RWLock lock(cmdPoolMutex);
        for (size_t inFlightFrameIdx = 0; inFlightFrameIdx < NumMaxInFlightFrames; ++inFlightFrameIdx)
        {
            auto* newCmdPool = new CommandPool(vulkanContext, queueType);
            localCmdPools[queueType][inFlightFrameIdx] = newCmdPool;
            cmdPools[inFlightFrameIdx].emplace_back(newCmdPool);
        }
    }

    return *(localCmdPools[queueType][frameTracker.GetCurrentInFlightFrameIndex()]);
}

void CommandPoolAllocator::BeginFrame()
{
    const auto& frameDependCmdPools = cmdPools[frameTracker.GetCurrentInFlightFrameIndex()];
    for (const auto& cmdPool : frameDependCmdPools)
    {
        cmdPool->BeginFrame();
    }
}

void CommandPoolAllocator::EndFrame()
{
}
} // namespace sy::vk
