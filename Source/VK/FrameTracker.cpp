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
        frame.CommandExecutionSemaphore = std::make_unique<Semaphore>(std::format("Cmd Execution Semaphore {}", frameIdx), vulkanContext);
        frame.SwapchainSemaphore = std::make_unique<Semaphore>(std::format("Swapchain Semaphore {}", frameIdx), vulkanContext, true);
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
        frame.CommandExecutionSemaphore.reset();
        frame.SwapchainSemaphore.reset();
        frame.PresentSemaphore.reset();
        frame.UploadSemaphore.reset();
    }
}

void FrameTracker::BeginFrame()
{
    /* Empty */
}

void FrameTracker::EndFrame()
{
    ++frameCounter;
}

Semaphore& FrameTracker::GetInflightCommandExecutionSemaphore()
{
    return *frames[GetFrameIndex()].CommandExecutionSemaphore;
}

Semaphore& FrameTracker::GetInflightSwapchainSemaphore()
{
    return *frames[GetFrameIndex()].SwapchainSemaphore;
}

Semaphore& FrameTracker::GetInflightPresentSemaphore()
{
    return *frames[GetFrameIndex()].PresentSemaphore;
}

Semaphore& FrameTracker::GetCurrentInFlightUploadSemaphore()
{
    return *frames[GetFrameIndex()].UploadSemaphore;
}


} // namespace sy::vk
