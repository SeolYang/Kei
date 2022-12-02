#pragma once
#include <Core.h>

namespace sy
{
	class Fence;
	class Semaphore;
	class FrameTracker
	{
	private:
		struct Frame
		{
			size_t InFlightFrameIndex = std::numeric_limits<size_t>::max();
			std::unique_ptr<Fence> RenderFence;
			std::unique_ptr<Semaphore> RenderSemaphore;
			std::unique_ptr<Semaphore> PresentSemaphore;
		};

	public:
		explicit FrameTracker(const VulkanContext& vulkanContext);
		~FrameTracker();

		FrameTracker(const FrameTracker&) = delete;
		FrameTracker(FrameTracker&&) = delete;
		FrameTracker& operator=(const FrameTracker&) = delete;
		FrameTracker& operator=(FrameTracker&&) = delete;

		void BeginFrame();
		void EndFrame();

		void WaitForInFlightRenderFence() const;
		void ResetInFlightRenderFence() const;

		Fence& GetCurrentInFlightRenderFence() const;
		Semaphore& GetCurrentInFlightRenderSemaphore() const;
		Semaphore& GetCurrentInFlightPresentSemaphore() const;

		[[nodiscard]] size_t GetCurrentFrameIndex() const { return currentFrameIdx; }
		[[nodiscard]] size_t GetCurrentInFlightFrameIndex() const { return currentFrameIdx % NumMaxInFlightFrames; }

	private:
		const VulkanContext& vulkanContext;
		std::array<Frame, NumMaxInFlightFrames> frames;
		size_t currentFrameIdx = 0;

	};
}