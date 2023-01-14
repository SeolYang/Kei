#pragma once
#include <PCH.h>

namespace sy::vk
{
	class VulkanContext;
	class CommandPoolManager;
	class CommandBuffer;
	class DescriptorManager;
	class FrameTracker;
	class Pipeline;
}

namespace sy::render
{
	class RenderPass : public NamedType, public NonCopyable
	{
	public:
		RenderPass(std::string_view name, const vk::VulkanContext& vulkanContext, vk::DescriptorManager& descriptorManager, const vk::FrameTracker& frameTracker, vk::CommandPoolManager& cmdPoolManager, const vk::Pipeline& pipeline);
		virtual ~RenderPass() noexcept override = default;

		RenderPass(const RenderPass&) = delete;
		RenderPass(RenderPass&&) = delete;

		RenderPass& operator=(const RenderPass&) = delete;
		RenderPass& operator=(RenderPass&&) = delete;

		virtual void UpdateBuffers() { }
		virtual vk::ManagedCommandBuffer Render() = 0;

		[[nodiscard]] const auto& GetVulkanContext() const { return vulkanContext; }
		[[nodiscard]] auto& GetDescriptorManager() const { return descriptorManager; }
		[[nodiscard]] const auto& GetFrameTracker() const { return frameTracker; }
		[[nodiscard]] const auto& GetPipeline() const { return pipeline; }
		[[nodiscard]] auto& GetCommandPoolManager() const { return cmdPoolManager; }

	private:
		const vk::VulkanContext& vulkanContext;
		vk::DescriptorManager& descriptorManager;
		const vk::FrameTracker& frameTracker;
		vk::CommandPoolManager& cmdPoolManager;
		const vk::Pipeline& pipeline;

	};
}