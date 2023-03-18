#pragma once
#include <PCH.h>

namespace sy::vk
{
class VulkanRHI;
class CommandPool;
class CommandBuffer;
class FrameTracker;
class CommandPoolManager final : public NonCopyable
{
public:
    CommandPoolManager(VulkanContext& vulkanContext, const FrameTracker& frameTracker);
    virtual ~CommandPoolManager() override;

    void Startup();
    void Shutdown();

    [[nodiscard]] CommandPool& RequestCommandPool(EQueueType queueType);

    void BeginFrame();
    void EndFrame();

private:
    VulkanContext&      vulkanContext;
    const FrameTracker& frameTracker;

    std::shared_mutex cmdPoolMutex;

    std::array<std::vector<std::unique_ptr<CommandPool>>, NumMaxInFlightFrames> cmdPools;
};
} // namespace sy::vk
