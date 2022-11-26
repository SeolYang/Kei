#pragma once
#include <Core.h>

namespace sy
{
	class VulkanInstance;
	class Window;
	class Semaphore;
	class Fence;

	struct Frame
	{
		std::unique_ptr<Fence> renderFence;
		std::unique_ptr<Semaphore> renderSemaphore;
		std::unique_ptr<Semaphore> presentSemaphore;
	};

	constexpr size_t NumMaxInFlightFrames = 2;

	class Renderer
	{
	public:
		Renderer(const Window& window, VulkanInstance& vulkanInstance);
		~Renderer();

		void Render();

	private:
		[[nodiscard]] const Frame& GetCurrentFrame() const { return frames[currentFrames % NumMaxInFlightFrames]; }
		const Frame& FrameBegin();
		void FrameEnd(const Frame& currentFrame);

	private:
		const Window& window;
		VulkanInstance& vulkanInstance;
		std::array<Frame, NumMaxInFlightFrames> frames;
		size_t currentFrames = 0;

	};
}