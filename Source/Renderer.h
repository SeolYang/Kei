#pragma once
#include <Core.h>

namespace sy
{
	class VulkanInstance;
	class Window;
	class Semaphore;
	class Fence;
	class Renderer
	{
	public:
		Renderer(const Window& window, VulkanInstance& vulkanInstance);
		~Renderer();

		void Render();

	private:
		const Window& window;
		VulkanInstance& vulkanInstance;
		std::unique_ptr<Fence> renderFence;
		std::unique_ptr<Semaphore> renderSemaphore;
		std::unique_ptr<Semaphore> presentSemaphore;

		size_t currentFrames = 0;

	};
}