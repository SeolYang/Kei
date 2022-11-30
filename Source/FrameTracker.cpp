#include <Core.h>
#include <FrameTracker.h>
#include <VK/Fence.h>
#include <VK/Semaphore.h>

namespace sy
{
	FrameTracker::FrameTracker(const VulkanContext& vulkanContext) :
		vulkanContext(vulkanContext)
	{
		for (size_t frameIdx = 0; frameIdx < NumMaxInFlightFrames; ++frameIdx)
		{
			frames[frameIdx] =
			{
				.InFlightFrameIndex = frameIdx,
				.RenderFence = std::make_unique<Fence>(std::format("Render Fence {}", frameIdx), vulkanContext),
				.RenderSemaphore = std::make_unique<Semaphore>(std::format("Render Semaphore {}", frameIdx), vulkanContext),
				.PresentSemaphore = std::make_unique<Semaphore>(std::format("Present Semaphore {}", frameIdx), vulkanContext)
			};
		}
	}

	FrameTracker::~FrameTracker()
	{
		/* Empty */
	}

	void FrameTracker::BeginFrame()
	{
		/* Empty */
	}

	void FrameTracker::EndFrame()
	{
		++currentFrameIdx;
	}

	void FrameTracker::WaitForInFlightRenderFence() const
	{
		const auto& fence = GetCurrentInFlightRenderFence();
		fence.Wait();
	}

	void FrameTracker::ResetInFlightRenderFence() const
	{
		const auto& fence = GetCurrentInFlightRenderFence();
		fence.Reset();
	}

	Fence& FrameTracker::GetCurrentInFlightRenderFence() const
	{
		return *frames[GetCurrentInFlightFrameIndex()].RenderFence;
	}

	Semaphore& FrameTracker::GetCurrentInFlightRenderSemaphore() const
	{
		return *frames[GetCurrentInFlightFrameIndex()].RenderSemaphore;
	}

	Semaphore& FrameTracker::GetCurrentInFlightPresentSemaphore() const
	{
		return *frames[GetCurrentInFlightFrameIndex()].PresentSemaphore;
	}
}
