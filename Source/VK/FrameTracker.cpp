#include <PCH.h>
#include <VK/FrameTracker.h>
#include <VK/VulkanContext.h>
#include <VK/Fence.h>
#include <VK/Semaphore.h>

namespace sy::vk
{
FrameTracker::FrameTracker(VulkanContext& vulkanContext) :
    vulkanContext(vulkanContext)
{
}

FrameTracker::~FrameTracker()
{
    /* Empty */
}

void FrameTracker::Startup()
{
    spdlog::info("Startup Frame Tracker.");
    const auto& vulkanRHI = vulkanContext.GetRHI();
    size_t frameIdx = 0;
    for (auto& frame : frames)
    {
        frame.InFlightFrameIndex = frameIdx,
        frame.RenderSemaphore = std::make_unique<Semaphore>(std::format("Render Semaphore {}", frameIdx), vulkanContext);
        frame.PresentSemaphore = std::make_unique<Semaphore>(std::format("Present Semaphore {}", frameIdx), vulkanContext, true);
        frame.UploadSemaphore = std::make_unique<Semaphore>(std::format("Upload Semaphore {}", frameIdx), vulkanContext);
        ++frameIdx;
    }
}

void FrameTracker::Shutdown()
{
    spdlog::info("Shutdown Frame Tracker.");
    for (auto& frame : frames)
    {
        frame.PresentSemaphore.reset();
    }
}

void FrameTracker::BeginFrame()
{
    /* Empty */
}

void FrameTracker::EndFrame()
{
    ++currentFrameIdx;
}

Semaphore& FrameTracker::GetCurrentInFlightRenderSemaphore() const
{
    return *frames[GetCurrentInFlightFrameIndex()].RenderSemaphore;
}

Semaphore& FrameTracker::GetCurrentInFlightPresentSemaphore() const
{
    return *frames[GetCurrentInFlightFrameIndex()].PresentSemaphore;
}

Semaphore& FrameTracker::GetCurrentInFlightUploadSemaphore() const
{
    return *frames[GetCurrentInFlightFrameIndex()].UploadSemaphore;
}

} // namespace sy::vk
