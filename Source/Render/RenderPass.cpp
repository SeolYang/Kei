#include <PCH.h>
#include <Render/RenderPass.h>
#include <VK/VulkanContext.h>
#include <VK/CommandPoolAllocator.h>
#include <VK/CommandPool.h>
#include <VK/CommandBuffer.h>

namespace sy::render
{
RenderPass::RenderPass(std::string_view name, vk::VulkanContext& vulkanContext, const vk::Pipeline& pipeline) :
    NamedType(name), vulkanContext(vulkanContext), pipeline(pipeline)
{
}

void RenderPass::Begin(const vk::EQueueType queueType)
{
    auto& cmdPoolAllocator = vulkanContext.GetCommandPoolAllocator();
    auto& graphicsCmdPool = cmdPoolAllocator.RequestCommandPool(queueType);
    currentCmdBuffer = graphicsCmdPool.RequestCommandBuffer(std::format("{}_CommandBuffer", GetName()));
    currentCmdBuffer->Begin();
    OnBegin();
}

void RenderPass::End()
{
    OnEnd();
    currentCmdBuffer->End();
}
} // namespace sy::render
