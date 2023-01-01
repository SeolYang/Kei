#pragma once
#include <Core/Core.h>

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
		RenderPass(std::string_view name, const vk::VulkanContext& vulkanContext, vk::DescriptorManager& descriptorManager, const vk::FrameTracker& frameTracker, const vk::Pipeline& pipeline);
		virtual ~RenderPass() noexcept override = default;

		RenderPass(const RenderPass&) = delete;
		RenderPass(RenderPass&&) = delete;

		RenderPass& operator=(const RenderPass&) = delete;
		RenderPass& operator=(RenderPass&&) = delete;

		virtual void UpdateBuffers() { }
		virtual vk::ManagedCommandBuffer Render(vk::CommandPoolManager& cmdPoolManager) = 0;

		const auto& GetVulkanContext() const { return vulkanContext; }
		auto& GetDescriptorManager() const { return descriptorManager; }
		const auto& GetFrameTracker() const { return frameTracker; }
		const auto& GetPipeline() const { return pipeline; }

	private:
		const vk::VulkanContext& vulkanContext;
		vk::DescriptorManager& descriptorManager;
		const vk::FrameTracker& frameTracker;
		const vk::Pipeline& pipeline;

	};
}