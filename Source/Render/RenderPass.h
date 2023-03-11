#pragma once
#include <PCH.h>

namespace sy::vk
{
	class VulkanRHI;
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
		RenderPass(std::string_view name, const vk::VulkanContext& vulkanContext, const vk::Pipeline& pipeline);
		virtual ~RenderPass() noexcept override = default;

		RenderPass(const RenderPass&) = delete;
		RenderPass(RenderPass&&) = delete;

		RenderPass& operator=(const RenderPass&) = delete;
		RenderPass& operator=(RenderPass&&) = delete;

		virtual void UpdateBuffers() { }

		void Begin(vk::EQueueType queueType);
		virtual void OnBegin() {}
		virtual void Render() = 0;
		void End();
		virtual void OnEnd() {}

		[[nodiscard]] const auto& GetVulkanContext() const { return vulkanContext; }
		[[nodiscard]] const auto& GetVulkanRHI() const { return GetVulkanContext().GetVulkanRHI(); }
		[[nodiscard]] auto& GetDescriptorManager() const { return GetVulkanContext().GetDescriptorManager(); }
		[[nodiscard]] const auto& GetFrameTracker() const { return GetVulkanContext().GetFrameTracker(); }
		[[nodiscard]] const auto& GetPipeline() const { return pipeline; }
		[[nodiscard]] auto& GetCommandPoolManager() const { return GetVulkanContext().GetCommandPoolManager(); }
		[[nodiscard]] auto& GetCommandBuffer() const { return *currentCmdBuffer; }

	private:
		const vk::VulkanContext& vulkanContext;
		const vk::Pipeline& pipeline;

		vk::ManagedCommandBuffer currentCmdBuffer;

	};
}