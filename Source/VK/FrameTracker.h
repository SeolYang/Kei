#pragma once
#include <PCH.h>

namespace sy::vk
{
class Fence;
class Semaphore;
class VulkanContext;
class FrameTracker : public NonCopyable
{
private:
    struct Frame
    {
        size_t InFlightFrameIndex = std::numeric_limits<size_t>::max();
        std::unique_ptr<Semaphore> RenderSemaphore;
        std::unique_ptr<Semaphore> PresentSemaphore;
        std::unique_ptr<Semaphore> UploadSemaphore;
    };

public:
    explicit FrameTracker(VulkanContext& vulkanContext);
    ~FrameTracker();

    void Startup();
    void Shutdown();

    void BeginFrame();
    void EndFrame();

    const Frame& GetCurrentInFlightFrame() const
    {
        return frames[GetCurrentInFlightFrameIndex()];
    }

    Semaphore& GetCurrentInFlightRenderSemaphore() const;
    Semaphore& GetCurrentInFlightPresentSemaphore() const;
    Semaphore& GetCurrentInFlightUploadSemaphore() const;

    [[nodiscard]] size_t GetCurrentFrameIndex() const
    {
        return currentFrameIdx;
    }

    [[nodiscard]] size_t GetCurrentInFlightFrameIndex() const
    {
        return currentFrameIdx % NumMaxInFlightFrames;
    }

private:
    VulkanContext& vulkanContext;
    std::array<Frame, NumMaxInFlightFrames> frames;
    size_t currentFrameIdx = 0;
};
} // namespace sy::vk
