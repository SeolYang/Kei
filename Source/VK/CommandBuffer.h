#pragma once
#include <PCH.h>

namespace sy
{
	namespace vk
	{
		class VulkanContext;
		class CommandPool;
		class Fence;
		class Pipeline;
		class Buffer;
		class Texture;
		class CommandBuffer : public VulkanWrapper<VkCommandBuffer>
		{
		public:
			CommandBuffer(std::string_view name, const VulkanContext& vulkanContext, const CommandPool& cmdPool);
			virtual ~CommandBuffer() override = default;

			[[nodiscard]] EQueueType GetQueueType() const { return queueType; }

			void Reset() const;
			void Begin() const;
			void End() const;

			void BeginRendering(const VkRenderingInfo& renderingInfo) const;
			void EndRendering() const;

			void ChangeImageAccessPattern(EAccessPattern srcAccessPattern, EAccessPattern dstAccessPattern, VkImage image, VkImageAspectFlags aspectMask, uint32_t mipLevelCount = 1, uint32_t baseMipLevel = 0, uint32_t arrayLayerCount = 1, uint32_t baseArrayLayer = 0) const;
			void ImageMemoryBarrier(VkPipelineStageFlags2 srcStage, VkPipelineStageFlags2 dstStage, VkAccessFlags2 srcAccess, VkAccessFlags2 dstAccess, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, VkImageAspectFlags aspectMask, uint32_t mipLevelCount = 1, uint32_t baseMipLevel = 0, uint32_t arrayLayerCount = 1, uint32_t baseArrayLayer = 0) const;

			void PipelineBarrier(std::span<VkMemoryBarrier2> memoryBarriers, std::span<VkBufferMemoryBarrier2> bufferMemoryBarriers, std::span<VkImageMemoryBarrier2> imageMemoryBarriers) const;

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

			void CopyBufferToImage(const Buffer& srcBuffer, const Texture& dstTexture, std::span<VkBufferImageCopy> copySubresourceRegions) const;
			void CopyBufferToImageSimple(const Buffer& srcBuffer, const Texture& dstTexture) const;
			void CopyBufferSimple(const Buffer& srcBuffer, size_t srcOffset, const Buffer& dstBuffer, size_t dstOffset, const size_t sizeofData) const;

		private:
			const EQueueType queueType;

		};
	}
}
