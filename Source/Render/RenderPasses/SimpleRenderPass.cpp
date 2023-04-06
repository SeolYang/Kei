#include <PCH.h>
#include <Render/RenderPasses/SimpleRenderPass.h>
#include <VK/VulkanContext.h>
#include <VK/CommandBuffer.h>
#include <VK/CommandPool.h>
#include <VK/CommandPoolAllocator.h>
#include <VK/VulkanRHI.h>
#include <VK/Buffer.h>
#include <VK/Texture.h>
#include <VK/TextureView.h>
#include <VK/DescriptorAllocator.h>
#include <VK/FrameTracker.h>
#include <VK/Swapchain.h>
#include <VK/Semaphore.h>
#include <VK/Sampler.h>
#include <VK/Fence.h>
#include <Render/Mesh.h>

namespace sy::render
{
SimpleRenderPass::SimpleRenderPass(const std::string_view name,
                                   vk::VulkanContext& vulkanContext,
                                   const vk::Pipeline& pipeline) :
    RenderPass(name, vulkanContext, pipeline)
{
    auto& cmdPoolAllocator = vulkanContext.GetCommandPoolAllocator();
    const auto& frameTracker = vulkanContext.GetFrameTracker();
    auto& descriptorAllocator = vulkanContext.GetDescriptorAllocator();

    vk::BufferBuilder transformBufferBuilder{vulkanContext};
    transformBufferBuilder.SetUsage(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT)
        .SetMemoryUsage(VMA_MEMORY_USAGE_CPU_TO_GPU)
        .SetSize<TransformUniformBuffer>();

    for (size_t idx = 0; idx < vk::NumMaxInFlightFrames; ++idx)
    {
        transformBufferBuilder.SetName(std::format("SimpleRenderPass_Transform_Buffer_{}", idx));
        transformBuffers[idx] = transformBufferBuilder.Build();

        auto& graphicsCmdPool = cmdPoolAllocator.RequestCommandPool(vk::EQueueType::Graphics);
        auto graphicsCmdBuffer = graphicsCmdPool.RequestCommandBuffer("Simple Render Pass Initial Sync");
        graphicsCmdBuffer->Begin();
        graphicsCmdBuffer->ChangeBufferState(vk::EBufferState::None, vk::EBufferState::VertexShaderReadUniformBuffer,
                                             *transformBuffers[idx]);
        graphicsCmdBuffer->End();

        const auto& uploadFence = frameTracker.GetCurrentInFlightUploadFence();
        vulkanContext.GetRHI().SubmitTo(*graphicsCmdBuffer, uploadFence);
        uploadFence.Wait();
        uploadFence.Reset();

        transformBufferIndices[idx] = descriptorAllocator.RequestDescriptor(*transformBuffers[idx]);
    }
}

void SimpleRenderPass::OnBegin()
{
    const auto& graphicsCmdBuffer = GetCommandBuffer();
    graphicsCmdBuffer.ChangeTextureState(vk::ETextureState::None, vk::ETextureState::ColorAttachmentWrite, swapchainImage,
                                         VK_IMAGE_ASPECT_COLOR_BIT);

    std::array colorAttachmentInfos = {swapchainAttachmentInfo};
    std::array depthAttachmentInfos = {depthAttachmentInfo};

    const VkRenderingInfo renderingInfo{
        .sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR,
        .pNext = nullptr,
        .renderArea = VkRect2D{
            .offset = VkOffset2D{0, 0},
            .extent = VkExtent2D{windowExtent.width, windowExtent.height},
        },
        .layerCount = 1,
        .colorAttachmentCount = static_cast<uint32_t>(colorAttachmentInfos.size()),
        .pColorAttachments = colorAttachmentInfos.data(),
        .pDepthAttachment = depthAttachmentInfos.data(),
        .pStencilAttachment = depthAttachmentInfos.data()};

    const auto& vulkanContext = GetVulkanContext();
    const auto& descriptorAllocator = vulkanContext.GetDescriptorAllocator();
    const auto& pipeline = GetPipeline();
    graphicsCmdBuffer.BeginRendering(renderingInfo);
    graphicsCmdBuffer.BindPipeline(pipeline);
    graphicsCmdBuffer.BindDescriptorSet(descriptorAllocator.GetDescriptorSet(), pipeline);
}

void SimpleRenderPass::Render()
{
    const auto& vulkanContext = GetVulkanContext();
    const auto& frameTracker = vulkanContext.GetFrameTracker();
    const auto& graphicsCmdBuffer = GetCommandBuffer();
    const auto& pipeline = GetPipeline();

    const PushConstants pushConstants{
        .textureIndex = static_cast<int>((*descriptor)->Offset),
        .transformDataIndex = static_cast<int>(transformBufferIndices[frameTracker.GetCurrentInFlightFrameIndex()]
                                                   ->Offset)};

    std::array vertexBuffers = {CRef<vk::Buffer>(mesh->GetVertexBuffer())};
    std::array offsets = {uint64_t()};

    graphicsCmdBuffer.BindVertexBuffers(0, vertexBuffers, offsets);
    graphicsCmdBuffer.BindIndexBuffer(mesh->GetIndexBuffer());
    graphicsCmdBuffer.PushConstants(pipeline, VK_SHADER_STAGE_ALL_GRAPHICS, pushConstants);

    graphicsCmdBuffer.DrawIndexed(static_cast<uint32_t>(mesh->GetNumIndices()), 1, 0, 0, 0);
}

void SimpleRenderPass::OnEnd()
{
    const auto& graphicsCmdBuffer = GetCommandBuffer();
    graphicsCmdBuffer.EndRendering();
    graphicsCmdBuffer.ChangeTextureState(vk::ETextureState::ColorAttachmentWrite, vk::ETextureState::Present,
                                         swapchainImage, VK_IMAGE_ASPECT_COLOR_BIT);
}

void SimpleRenderPass::UpdateBuffers()
{
    const auto& vulkanContext = GetVulkanContext();
    const auto& vulkanRHI = vulkanContext.GetRHI();
    const auto& frameTracker = vulkanContext.GetFrameTracker();
    const auto& transformBuffer = *transformBuffers[frameTracker.GetCurrentInFlightFrameIndex()];
    void* transformBufferMappedPtr = vulkanRHI.Map(transformBuffer);
    memcpy(transformBufferMappedPtr, &transformData, sizeof(TransformUniformBuffer));
    vulkanRHI.Unmap(transformBuffer);
}

void SimpleRenderPass::SetMesh(Handle<Mesh> mesh)
{
    this->mesh = mesh;
}

void SimpleRenderPass::SetTextureDescriptor(const Handle<vk::Descriptor> descriptor)
{
    this->descriptor = descriptor;
}

void SimpleRenderPass::SetWindowExtent(Extent2D<uint32_t> extent)
{
    this->windowExtent = extent;
}

void SimpleRenderPass::SetSwapchain(const vk::Swapchain& swapchain, VkClearColorValue clearColorValue)
{
    swapchainImage = swapchain.GetCurrentImage();
    swapchainAttachmentInfo = swapchain.GetColorAttachmentInfo(clearColorValue);
}

void SimpleRenderPass::SetDepthStencilView(const vk::TextureView& depthStencilView)
{
    depthAttachmentInfo = vk::DepthAttachmentInfo(depthStencilView);
}

void SimpleRenderPass::SetTransformData(const TransformUniformBuffer buffer)
{
    transformData = buffer;
}
} // namespace sy::render
