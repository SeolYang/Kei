#pragma once
#include <Core.h>

namespace sy
{
	class VulkanInstance;
	class Window;
	class Semaphore;
	class Fence;
	class ShaderModule;
	class Pipeline;
	class DescriptorPool;
	struct Frame
	{
		size_t inFlightFrameIdx;
		std::unique_ptr<Fence> renderFence;
		std::unique_ptr<Semaphore> renderSemaphore;
		std::unique_ptr<Semaphore> presentSemaphore;
		std::unique_ptr<DescriptorPool> globalDescriptorPool;

	};

	class Renderer
	{
	public:
		Renderer(const Window& window, VulkanInstance& vulkanInstance);
		~Renderer();

		void Render();

		[[nodiscard]] size_t GetCurrentInFlightFrameIndex() const { return currentFrameIdx % NumMaxInFlightFrames; }
		[[nodiscard]] const Frame& GetCurrentInFlightFrame() const { return frames[GetCurrentInFlightFrameIndex()]; }
		[[nodiscard]] size_t GetCurrentFrameIndex() const { return currentFrameIdx; }

	private:
		const Frame& BeginFrame();
		void EndFrame(const Frame& currentFrame);

	private:
		const Window& window;
		VulkanInstance& vulkanInstance;
		std::array<Frame, NumMaxInFlightFrames> frames;
		size_t currentFrameIdx = 0;

		std::unique_ptr<ShaderModule> triVert;
		std::unique_ptr<ShaderModule> triFrag;
		std::unique_ptr<Pipeline> basicPipeline;
		VkPipelineLayout nullLayout;

	};
}