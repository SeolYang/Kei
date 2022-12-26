#pragma once
#include <Core/Core.h>

namespace sy::vk
{
	class VulkanContext;
	class CommandPoolManager;
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

		virtual void PreRender(vk::CommandPoolManager& cmdPoolManager) = 0;
		virtual void Render(vk::CommandPoolManager& cmdPoolManager) = 0;
		virtual void PostRender(vk::CommandPoolManager& cmdPoolManager) = 0;

	protected:
		const vk::VulkanContext& vulkanContext;
		vk::DescriptorManager& descriptorManager;
		const vk::FrameTracker& frameTracker;
		const vk::Pipeline& pipeline;

	};
}