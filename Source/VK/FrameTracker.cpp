#include <PCH.h>
#include <VK/FrameTracker.h>
#include <VK/Fence.h>
#include <VK/Semaphore.h>

namespace sy
{
	namespace vk
	{
		FrameTracker::FrameTracker(const VulkanRHI& vulkanRHI) :
			vulkanRHI(vulkanRHI)
		{
			for (size_t frameIdx = 0; frameIdx < NumMaxInFlightFrames; ++frameIdx)
			{
				frames[ frameIdx ] =
				{
					.InFlightFrameIndex = frameIdx,
					.RenderFence = std::make_unique<Fence>(std::format("Render Fence {}", frameIdx), vulkanRHI),
					.UploadFence = std::make_unique<Fence>(std::format("Upload Fence {}", frameIdx), vulkanRHI, false),
					.RenderSemaphore = std::make_unique<Semaphore>(std::format("Render Semaphore {}", frameIdx),
					                                               vulkanRHI),
					.PresentSemaphore = std::make_unique<Semaphore>(std::format("Present Semaphore {}", frameIdx),
					                                                vulkanRHI),
					.UploadSemaphore = std::make_unique<Semaphore>(std::format("Upload Semaphore {}", frameIdx),
					                                               vulkanRHI),
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
			return *frames[ GetCurrentInFlightFrameIndex() ].RenderFence;
		}

		Fence& FrameTracker::GetCurrentInFlightUploadFence() const
		{
			return *frames[ GetCurrentInFlightFrameIndex() ].UploadFence;
		}

		Semaphore& FrameTracker::GetCurrentInFlightRenderSemaphore() const
		{
			return *frames[ GetCurrentInFlightFrameIndex() ].RenderSemaphore;
		}

		Semaphore& FrameTracker::GetCurrentInFlightPresentSemaphore() const
		{
			return *frames[ GetCurrentInFlightFrameIndex() ].PresentSemaphore;
		}

		Semaphore& FrameTracker::GetCurrentInFlightUploadSemaphore() const
		{
			return *frames[ GetCurrentInFlightFrameIndex() ].UploadSemaphore;
		}
	}
}
