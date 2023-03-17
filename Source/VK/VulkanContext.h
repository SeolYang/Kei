#pragma once
#include <PCH.h>

namespace sy::window
{
	class Window;
}

namespace sy::vk
{
	class VulkanRHI;
	class CommandPoolManager;
	class DescriptorManager;
	class FrameTracker;
	class PipelineLayoutCache;

	class VulkanContext
	{
	public:
		VulkanContext(const window::Window& window);
		~VulkanContext();

		[[nodiscard]] VulkanRHI& GetRHI() const;
		[[nodiscard]] FrameTracker& GetFrameTracker() const;
		[[nodiscard]] CommandPoolManager& GetCommandPoolManager() const;
		[[nodiscard]] DescriptorManager& GetDescriptorManager() const;
		[[nodiscard]] PipelineLayoutCache& GetPipelineLayoutCache() const;

		void BeginFrame();
		void EndFrame();

		void BeginRender();
		void EndRender();

	private:
		std::unique_ptr<VulkanRHI> vulkanRHI;
		std::unique_ptr<FrameTracker> frameTracker;
		std::unique_ptr<CommandPoolManager> cmdPoolManager;
		std::unique_ptr<DescriptorManager> descriptorManager;
		std::unique_ptr<PipelineLayoutCache> pipelineLayoutCache;
	};
} // namespace sy::vk
