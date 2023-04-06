#pragma once
#include <PCH.h>

namespace sy::vk
{
class VulkanRHI;
class CommandPool;
class CommandBuffer;
class FrameTracker;
class CommandPoolAllocator final : public NonCopyable
{
public:
    CommandPoolAllocator(VulkanContext& vulkanContext, const FrameTracker& frameTracker);
    virtual ~CommandPoolAllocator() override;

    void Startup();
    void Shutdown();

    [[nodiscard]] CommandPool& RequestCommandPool(EQueueType queueType);

    void BeginFrame();
    void EndFrame();

private:
    VulkanContext& vulkanContext;
    const FrameTracker& frameTracker;

    std::shared_mutex cmdPoolMutex;

    std::array<std::vector<std::unique_ptr<CommandPool>>, NumMaxInFlightFrames> cmdPools;
};
} // namespace sy::vk
