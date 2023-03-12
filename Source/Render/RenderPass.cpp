#include <PCH.h>
#include <Render/RenderPass.h>
#include <VK/VulkanContext.h>
#include <VK/CommandPoolManager.h>
#include <VK/CommandPool.h>
#include <VK/CommandBuffer.h>

namespace sy::render
{
	RenderPass::RenderPass(std::string_view name, const vk::VulkanContext& vulkanContext, const vk::Pipeline& pipeline) :
	NamedType(name),
	vulkanContext(vulkanContext),
	pipeline(pipeline)
	{
	}

	void RenderPass::Begin(const vk::EQueueType queueType)
	{
		auto& cmdPoolManager = vulkanContext.GetCommandPoolManager();
		auto& graphicsCmdPool = cmdPoolManager.RequestCommandPool(queueType);
		currentCmdBuffer = graphicsCmdPool.RequestCommandBuffer(std::format("{}_CommandBuffer", GetName()));;
		currentCmdBuffer->Begin();
		OnBegin();
	}

	void RenderPass::End()
	{
		OnEnd();
		currentCmdBuffer->End();
	}
}
