#pragma once
#include <PCH.h>
#include <VK/VulkanWrapper.h>
#include <VK/TextureStateTransition.h>
#include <VK/BufferStateTransition.h>

namespace sy::vk
{
class CommandPool;
class Fence;
class Pipeline;
class Buffer;
class Texture;
class CommandBuffer : public VulkanWrapper<VkCommandBuffer>
{
public:
    CommandBuffer(std::string_view name, VulkanContext& vulkanContext, const CommandPool& cmdPool);
    ~CommandBuffer() override = default;

    [[nodiscard]] EQueueType GetQueueType() const
    {
        return queueType;
    }

    void Reset() const;
    void Begin() const;
    void End() const;

    void BeginRendering(const VkRenderingInfo& renderingInfo) const;
    void EndRendering() const;

    void ApplyStateTransition(TextureStateTransition transition) const;
    void ApplyStateTransition(BufferStateTransition transition) const;
    void ApplyStateTransitions(std::span<const TextureStateTransition> transitions) const;
    void ApplyStateTransitions(std::span<const BufferStateTransition> transitions) const;

	void BatchStateTransition(TextureStateTransition transition);
    void BatchStateTransition(BufferStateTransition transition);
    void BatchStateTransitions(std::span<const TextureStateTransition> transitions);
    void BatchStateTransitions(std::span<const BufferStateTransition> transitions);
    void FlushBatchedStateTransitions();

    void BindPipeline(const Pipeline& pipeline) const;
    void BindDescriptorSet(VkDescriptorSet descriptorSet, const Pipeline& pipeline) const;
    void BindVertexBuffers(uint32_t firstBinding, std::span<CRef<Buffer>> buffers, std::span<size_t> offsets) const;
    void BindVertexBuffers(uint32_t firstBinding, std::span<VkBuffer> buffers, std::span<size_t> offsets) const;
    void BindIndexBuffer(const Buffer& indexBuffer, size_t offset = 0) const;
    void BindIndexBuffer(VkBuffer indexBuffer, size_t offset = 0) const;

    template <typename T>
    void PushConstants(const Pipeline& pipeline, const VkShaderStageFlags shaderStageFlags, const T& value) const
    {
        PushConstants(pipeline, shaderStageFlags, 0, sizeof(T), &value);
    }

    void PushConstants(const Pipeline& pipeline, VkShaderStageFlags shaderStageFlags, uint32_t offset, uint32_t size, const void* values) const;

    void Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) const;
    void DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance) const;

    void CopyBufferToImage(const Buffer& srcBuffer, const Texture& dstTexture, std::span<const VkBufferImageCopy> copySubresourceRegions) const;
    void CopyBufferToImageSimple(const Buffer& srcBuffer, const Texture& dstTexture) const;
    void CopyBufferSimple(const Buffer& srcBuffer, size_t srcOffset, const Buffer& dstBuffer, size_t dstOffset, const size_t sizeofData) const;
    void CopyImageToBuffer(const Texture& srcTexture, const Buffer& dstBuffer) const;
    void BlitTexture(const Texture& src, const Texture& dst, VkImageBlit blit, VkFilter filter = VK_FILTER_LINEAR) const;

private:
    void PipelineBarrier(std::span<VkMemoryBarrier2> memoryBarriers,
                         std::span<VkBufferMemoryBarrier2> bufferMemoryBarriers,
                         std::span<VkImageMemoryBarrier2> imageMemoryBarriers) const;

private:
    const EQueueType queueType;
    std::vector<TextureStateTransition> batchedTextureStateTransitions;
    std::vector<BufferStateTransition> batchedBufferStateTransitions;
};
} // namespace sy::vk
