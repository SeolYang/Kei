#include <PCH.h>
#include <VK/CommandBuffer.h>
#include <VK/CommandPool.h>
#include <VK/VulkanRHI.h>
#include <VK/Pipeline.h>
#include <VK/Buffer.h>
#include <VK/Texture.h>

namespace sy::vk
{
CommandBuffer::CommandBuffer(const std::string_view name, VulkanContext& vulkanContext, const CommandPool& cmdPool) :
    VulkanWrapper<VkCommandBuffer>(name, vulkanContext, VK_OBJECT_TYPE_COMMAND_BUFFER), queueType(cmdPool.GetQueueType())
{
    const VkCommandBufferAllocateInfo allocInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = nullptr,
        .commandPool = cmdPool.GetNative(),
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1};

    NativeHandle handle = VK_NULL_HANDLE;
    VK_ASSERT(vkAllocateCommandBuffers(GetRHI().GetDevice(), &allocInfo, &handle),
              "Failed to creating command buffer.");
    UpdateHandle(handle, SY_VK_WRAPPER_EMPTY_DELETER);
}

void CommandBuffer::Reset() const
{
    vkResetCommandBuffer(GetNative(), 0);
}

void CommandBuffer::Begin() const
{
    const VkCommandBufferBeginInfo beginInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        .pInheritanceInfo = nullptr};

    VK_ASSERT(vkBeginCommandBuffer(GetNative(), &beginInfo), "Faeild to begin command buffer {}.",
              GetName());
}

void CommandBuffer::End() const
{
    VK_ASSERT(vkEndCommandBuffer(GetNative()), "Failed to end command buffer {}.", GetName());
}

void CommandBuffer::BeginRendering(const VkRenderingInfo& renderingInfo) const
{
    vkCmdBeginRendering(GetNative(), &renderingInfo);
}

void CommandBuffer::EndRendering() const
{
    vkCmdEndRendering(GetNative());
}

void CommandBuffer::ApplyStateTransition(const TextureStateTransition transition) const
{
    VkImageMemoryBarrier2 barriers[] = {transition.Build()};
    PipelineBarrier({}, {}, barriers);
}

void CommandBuffer::ApplyStateTransition(BufferStateTransition transition) const
{
    VkBufferMemoryBarrier2 barriers[] = {transition.Build()};
    PipelineBarrier({}, barriers, {});
}

void CommandBuffer::ApplyStateTransitions(const std::span<const TextureStateTransition> transitions) const
{
    std::vector<VkImageMemoryBarrier2> barriers;
    barriers.reserve(transitions.size());
    std::transform(
		transitions.begin(), transitions.end(), 
		barriers.begin(), 
		[](const TextureStateTransition& transition) {
            return transition.Build();
    });

    PipelineBarrier({}, {}, barriers);
}

void CommandBuffer::ApplyStateTransitions(std::span<const BufferStateTransition> transitions) const
{
    std::vector<VkBufferMemoryBarrier2> barriers;
    barriers.reserve(transitions.size());
    std::transform(
        transitions.begin(), transitions.end(),
        barriers.begin(),
        [](const BufferStateTransition& transition) {
            return transition.Build();
        });

    PipelineBarrier({}, barriers, {});
}

void CommandBuffer::BindPipeline(const Pipeline& pipeline) const
{
    vkCmdBindPipeline(GetNative(), pipeline.GetBindPoint(), pipeline.GetNative());
}

void CommandBuffer::BindDescriptorSet(VkDescriptorSet descriptorSet, const Pipeline& pipeline) const
{
    const VkDescriptorSet descriptorSets[] = {
        descriptorSet,
    };
    vkCmdBindDescriptorSets(GetNative(), pipeline.GetBindPoint(), pipeline.GetLayout(), 0, 1,
                            descriptorSets, 0, nullptr);
}

void CommandBuffer::BindVertexBuffers(const uint32_t firstBinding, const std::span<CRef<Buffer>> buffers, const std::span<size_t> offsets) const
{
    std::vector<VkBuffer> handles;
    handles.resize(buffers.size());
    std::transform(buffers.begin(), buffers.end(),
                   handles.begin(),
                   [](const Buffer& buffer) {
                       return buffer.GetNative();
                   });

    BindVertexBuffers(firstBinding, handles, offsets);
}

void CommandBuffer::BindVertexBuffers(const uint32_t firstBinding, const std::span<VkBuffer> buffers, const std::span<size_t> offsets) const
{
    vkCmdBindVertexBuffers(GetNative(), firstBinding, static_cast<uint32_t>(buffers.size()),
                           buffers.data(), offsets.data());
}

void CommandBuffer::BindIndexBuffer(const Buffer& indexBuffer, const size_t offset) const
{
    BindIndexBuffer(indexBuffer.GetNative(), offset);
}

void CommandBuffer::BindIndexBuffer(const VkBuffer indexBuffer, const size_t offset) const
{
    vkCmdBindIndexBuffer(GetNative(), indexBuffer, offset, VK_INDEX_TYPE_UINT32);
}

void CommandBuffer::PushConstants(const Pipeline& pipeline, const VkShaderStageFlags shaderStageFlags, const uint32_t offset, const uint32_t size, const void* values) const
{
    vkCmdPushConstants(GetNative(), pipeline.GetLayout(), shaderStageFlags, offset, size, values);
}

void CommandBuffer::Draw(const uint32_t vertexCount, const uint32_t instanceCount, const uint32_t firstVertex, const uint32_t firstInstance) const
{
    vkCmdDraw(GetNative(), vertexCount, instanceCount, firstVertex, firstInstance);
}

void CommandBuffer::DrawIndexed(const uint32_t indexCount, const uint32_t instanceCount, const uint32_t firstIndex, const int32_t vertexOffset, const uint32_t firstInstance) const
{
    vkCmdDrawIndexed(GetNative(), indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

void CommandBuffer::CopyBufferToImage(const Buffer& srcBuffer, const Texture& dstTexture, const std::span<const VkBufferImageCopy> regions) const
{
    vkCmdCopyBufferToImage(GetNative(), srcBuffer.GetNative(), dstTexture.GetNative(),
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, static_cast<uint32_t>(regions.size()),
                           regions.data());
}

void CommandBuffer::CopyBufferToImageSimple(const Buffer& srcBuffer, const Texture& dstTexture) const
{
    const auto imageExtent = dstTexture.GetExtent();
    const VkBufferImageCopy imgCopy{
        .bufferOffset = 0,
        .bufferRowLength = 0,
        .bufferImageHeight = 0,
        .imageSubresource = {
            .aspectMask = FormatToImageAspect(dstTexture.GetFormat()),
            .mipLevel = 0,
            .baseArrayLayer = 0,
            .layerCount = 1},
        .imageExtent = {imageExtent.width, imageExtent.height, imageExtent.depth}};

    std::array region = {imgCopy};
    CopyBufferToImage(srcBuffer, dstTexture, region);
}

void CommandBuffer::CopyBufferSimple(const Buffer& srcBuffer, const size_t srcOffset, const Buffer& dstBuffer, const size_t dstOffset, const size_t sizeofData) const
{
    const VkBufferCopy bufferCopy{
        .srcOffset = srcOffset,
        .dstOffset = dstOffset,
        .size = sizeofData};

    vkCmdCopyBuffer(GetNative(), srcBuffer.GetNative(), dstBuffer.GetNative(), 1,
                    &bufferCopy);
}

void CommandBuffer::CopyImageToBuffer(const Texture& srcTexture, const Buffer& dstBuffer) const
{
    const auto imageExtent = srcTexture.GetExtent();
    const VkBufferImageCopy imgCopy{
        .bufferOffset = 0,
        .bufferRowLength = 0,
        .bufferImageHeight = 0,
        .imageSubresource = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .mipLevel = 0,
            .baseArrayLayer = 0,
            .layerCount = 1},
        .imageExtent = {imageExtent.width, imageExtent.height, imageExtent.depth}};

    vkCmdCopyImageToBuffer(GetNative(), srcTexture.GetNative(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dstBuffer.GetNative(), 1, &imgCopy);
}

void CommandBuffer::PipelineBarrier(std::span<VkMemoryBarrier2> memoryBarriers,
                                    std::span<VkBufferMemoryBarrier2> bufferMemoryBarriers,
                                    std::span<VkImageMemoryBarrier2> imageMemoryBarriers) const
{
    const VkDependencyInfo dependencyInfo{
        .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
        .pNext = nullptr,
        .memoryBarrierCount = static_cast<uint32_t>(memoryBarriers.size()),
        .pMemoryBarriers = memoryBarriers.data(),
        .bufferMemoryBarrierCount = static_cast<uint32_t>(bufferMemoryBarriers.size()),
        .pBufferMemoryBarriers = bufferMemoryBarriers.data(),
        .imageMemoryBarrierCount = static_cast<uint32_t>(imageMemoryBarriers.size()),
        .pImageMemoryBarriers = imageMemoryBarriers.data()};

    vkCmdPipelineBarrier2(GetNative(), &dependencyInfo);
}

void CommandBuffer::BlitTexture(const Texture& src, const Texture& dst, const VkImageBlit blit, const VkFilter filter) const
{
    vkCmdBlitImage(
        GetNative(),
        src.GetNative(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        dst.GetNative(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1, &blit,
        filter);
}

} // namespace sy::vk
