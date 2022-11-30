#pragma once
#include <Core.h>

namespace sy
{
	class VulkanContext;
	class CommandPool;
	class Fence;
	class Pipeline;
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

		void ChangeImageLayout(VkPipelineStageFlags srcStage, VkPipelineStageFlags dstStage, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, VkImageAspectFlags aspectMask, uint32_t mipLevelCount = 1, uint32_t baseMipLevel = 0, uint32_t arrayLayerCount = 1, uint32_t baseArrayLayer = 0) const;
		void PipelineBarrier(VkPipelineStageFlags srcStage, VkPipelineStageFlags dstStage, std::span<VkMemoryBarrier> memoryBarriers, std::span<VkBufferMemoryBarrier> bufferMemoryBarriers, std::span<VkImageMemoryBarrier> imageMemoryBarriers) const;

		void BindPipeline(const Pipeline& pipeline) const;
		void Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) const;

	public:
		static std::pair<VkAccessFlags, VkAccessFlags> QueryOptimalAccessFlagFromImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout);

	private:
		const EQueueType queueType;

	};

}
