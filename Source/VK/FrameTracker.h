#pragma once
#include <PCH.h>

namespace sy::vk
{
class Fence;
class Semaphore;
class VulkanContext;
// todo: Should i move frame tracker to render module?
class FrameTracker : public NonCopyable
{
private:
    struct Frame
    {
        size_t InFlightFrameIndex = std::numeric_limits<size_t>::max();
        std::unique_ptr<Semaphore> CommandExecutionSemaphore;
        std::unique_ptr<Semaphore> PresentSemaphore;
        std::unique_ptr<Semaphore> SwapchainSemaphore;
        std::unique_ptr<Semaphore> UploadSemaphore;
    };

public:
    explicit FrameTracker(VulkanContext& vulkanContext);
    ~FrameTracker();

    void Startup();
    void Shutdown();

    void BeginFrame();
    void EndFrame();

    const Frame& GetInflightFrame() const
    {
        return frames[GetFrameIndex()];
    }

    Semaphore& GetInflightCommandExecutionSemaphore();
    Semaphore& GetInflightSwapchainSemaphore();
    Semaphore& GetInflightPresentSemaphore();
    Semaphore& GetCurrentInFlightUploadSemaphore();

    [[nodiscard]] size_t GetFrameCounter() const
    {
        return frameCounter;
    }

    [[nodiscard]] size_t GetFrameIndex() const
    {
        return frameCounter % NumMaxInFlightFrames;
    }

private:
    VulkanContext& vulkanContext;
    std::array<Frame, NumMaxInFlightFrames> frames;
    size_t frameCounter = 0;
};
} // namespace sy::vk
