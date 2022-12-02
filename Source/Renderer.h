#pragma once
#include <Core.h>

namespace sy
{
	class VulkanContext;
	class Window;
	class Semaphore;
	class Fence;
	class ShaderModule;
	class Pipeline;
	class FrameTracker;
	class CommandPoolManager;
	class DescriptorManager;
	class Texture2D;
	class Renderer
	{
	public:
		Renderer(const Window& window, VulkanContext& vulkanContext, const FrameTracker& frameTracker, CommandPoolManager& cmdPoolManager, DescriptorManager& descriptorManager);
		~Renderer();

		void Render();

	private:
		void BeginFrame();
		void EndFrame();

	private:
		const Window& window;
		VulkanContext& vulkanContext;
		const FrameTracker& frameTracker;
		CommandPoolManager& cmdPoolManager;
		DescriptorManager& descriptorManager;
		
		std::unique_ptr<ShaderModule> triVert;
		std::unique_ptr<ShaderModule> triFrag;
		std::unique_ptr<Pipeline> basicPipeline;

		std::unique_ptr<Texture2D> test;

	};
}