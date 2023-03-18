#include <PCH.h>
#include <VK/FrameTracker.h>
#include <VK/VulkanContext.h>
#include <VK/Fence.h>
#include <VK/Semaphore.h>

namespace sy
{
	namespace vk
	{
		FrameTracker::FrameTracker(VulkanContext& vulkanContext)
			: vulkanContext(vulkanContext)
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
				frame.RenderFence = std::make_unique<Fence>(std::format("Render Fence {}", frameIdx), vulkanContext);
				frame.UploadFence = std::make_unique<Fence>(std::format("Upload Fence {}", frameIdx), vulkanContext, false);
				frame.RenderSemaphore = std::make_unique<Semaphore>(std::format("Render Semaphore {}", frameIdx), vulkanContext);
				frame.PresentSemaphore = std::make_unique<Semaphore>(std::format("Present Semaphore {}", frameIdx), vulkanContext);
				frame.UploadSemaphore = std::make_unique<Semaphore>(std::format("Upload Semaphore {}", frameIdx), vulkanContext);
				++frameIdx;
			}
		}

		void FrameTracker::Shutdown()
		{
			spdlog::info("Shutdown Frame Tracker.");
			for (auto& frame : frames)
			{
				frame.RenderFence.reset();
				frame.UploadFence.reset();
				frame.RenderSemaphore.reset();
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

		Fence& FrameTracker::GetCurrentInFlightUploadFence() const
		{
			return *frames[GetCurrentInFlightFrameIndex()].UploadFence;
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

	} // namespace vk
} // namespace sy
