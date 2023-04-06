#pragma once
#include <PCH.h>

namespace sy::vk
{
class VulkanRHI;
class VulkanContext;
class CommandPoolAllocator;
class CommandBuffer;
class DescriptorManager;
class FrameTracker;
class Pipeline;
} // namespace sy::vk

namespace sy::render
{
class RenderPass : public NamedType, public NonCopyable
{
public:
    RenderPass(std::string_view name, vk::VulkanContext& vulkanContext, const vk::Pipeline& pipeline);
    virtual ~RenderPass() noexcept override = default;

    RenderPass(const RenderPass&) = delete;
    RenderPass(RenderPass&&)      = delete;

    RenderPass& operator=(const RenderPass&) = delete;
    RenderPass& operator=(RenderPass&&)      = delete;

    virtual void UpdateBuffers()
    {
    }

    void Begin(vk::EQueueType queueType);

    virtual void OnBegin()
    {
    }

    virtual void Render() = 0;
    void         End();

    virtual void OnEnd()
    {
    }

    [[nodiscard]] const auto& GetVulkanContext() const
    {
        return vulkanContext;
    }

    [[nodiscard]] const auto& GetPipeline() const
    {
        return pipeline;
    }

    [[nodiscard]] auto& GetCommandBuffer() const
    {
        return *currentCmdBuffer;
    }

private:
    vk::VulkanContext&  vulkanContext;
    const vk::Pipeline& pipeline;

    vk::ManagedCommandBuffer currentCmdBuffer;
};
} // namespace sy::render
