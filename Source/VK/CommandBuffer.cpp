#include <Core.h>
#include <VK/CommandBuffer.h>
#include <VK/CommandPool.h>
#include <VK/VulkanContext.h>
#include <VK/Pipeline.h>
#include <VK/Buffer.h>
#include <VK/Texture.h>

namespace sy
{
	CommandBuffer::CommandBuffer(std::string_view name, const VulkanContext& vulkanContext, const CommandPool& cmdPool) :
		VulkanWrapper<VkCommandBuffer>(name, vulkanContext, VK_DESTROY_LAMBDA_SIGNATURE(VkCommandBuffer){ }),
		queueType(cmdPool.GetQueueType())
	{
		const VkCommandBufferAllocateInfo allocInfo
		{
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			.pNext = nullptr,
			.commandPool = cmdPool.GetNativeHandle(),
			.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			.commandBufferCount = 1
		};

		VK_ASSERT(vkAllocateCommandBuffers(vulkanContext.GetDevice(), &allocInfo, &handle), "Failed to creating command buffer.");
	}

	void CommandBuffer::Reset() const
	{
		vkResetCommandBuffer(handle, 0);
	}

	void CommandBuffer::Begin() const
	{
		const VkCommandBufferBeginInfo beginInfo
		{
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			.pNext = nullptr,
			.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
			.pInheritanceInfo = nullptr
		};

		VK_ASSERT(vkBeginCommandBuffer(handle, &beginInfo), "Faeild to begin command buffer {}.", GetName());
	}

	void CommandBuffer::End() const
	{
		VK_ASSERT(vkEndCommandBuffer(handle), "Failed to end command buffer {}.", GetName());
	}

	void CommandBuffer::BeginRendering(const VkRenderingInfo& renderingInfo) const
	{
		vkCmdBeginRendering(handle, &renderingInfo);
	}

	void CommandBuffer::EndRendering() const
	{
		vkCmdEndRendering(handle);
	}

	void CommandBuffer::ChangeImageLayout(VkPipelineStageFlags srcStage, VkPipelineStageFlags dstStage, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, VkImageAspectFlags aspectMask, uint32_t mipLevelCount, uint32_t baseMipLevel, uint32_t arrayLayerCount, uint32_t baseArrayLayer) const
	{
		SY_ASSERT(srcStage != dstStage, "Redundant image layout change detected.");
		const auto [srcAccess, dstAccess] = QueryOptimalAccessFlagFromImageLayout(oldLayout, newLayout);
		const VkImageMemoryBarrier barrier
		{
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
			.pNext = nullptr,
			.srcAccessMask = srcAccess,
			.dstAccessMask = dstAccess,
			.oldLayout = oldLayout,
			.newLayout = newLayout,
			.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.image = image,
			.subresourceRange = {
				.aspectMask = aspectMask,
				.baseMipLevel = baseMipLevel,
				.levelCount = mipLevelCount,
				.baseArrayLayer = baseArrayLayer,
				.layerCount = arrayLayerCount,
			}
		};

		VkImageMemoryBarrier barriers[] = { barrier };
		PipelineBarrier(srcStage, dstStage, {}, {}, barriers);
	}

	void CommandBuffer::PipelineBarrier(VkPipelineStageFlags srcStage, VkPipelineStageFlags dstStage, std::span<VkMemoryBarrier> memoryBarriers, std::span<VkBufferMemoryBarrier> bufferMemoryBarriers, std::span<VkImageMemoryBarrier> imageMemoryBarriers) const
	{
		vkCmdPipelineBarrier(handle,
			srcStage, dstStage, 0,
			memoryBarriers.size(), memoryBarriers.data(),
			bufferMemoryBarriers.size(), bufferMemoryBarriers.data(),
			imageMemoryBarriers.size(), imageMemoryBarriers.data());
	}

	void CommandBuffer::BindPipeline(const Pipeline& pipeline) const
	{
		vkCmdBindPipeline(handle, pipeline.GetBindPoint(), pipeline.GetNativeHandle());
	}

	void CommandBuffer::BindDescriptorSet(VkDescriptorSet descriptorSet, const Pipeline& pipeline) const
	{
		const VkDescriptorSet descriptorSets[] = { descriptorSet, };
		vkCmdBindDescriptorSets(handle, pipeline.GetBindPoint(), pipeline.GetLayout(), 0, 1, descriptorSets, 0, nullptr);
	}

	void CommandBuffer::BindVertexBuffers(const uint32_t firstBinding, const std::span<CRef<Buffer>> buffers, const std::span<size_t> offsets) const
	{
		std::vector<VkBuffer> handles;
		handles.resize(buffers.size());
		std::transform(buffers.begin(), buffers.end(),
			handles.begin(),
			[](const Buffer& buffer)
			{
				return buffer.GetNativeHandle();
			});

		vkCmdBindVertexBuffers(handle, firstBinding, handles.size(), handles.data(), offsets.data());
	}

	void CommandBuffer::BindIndexBuffer(const Buffer& indexBuffer, const size_t offset) const
	{
		vkCmdBindIndexBuffer(handle, indexBuffer.GetNativeHandle(), offset, VK_INDEX_TYPE_UINT32);
	}

	void CommandBuffer::PushConstants(const Pipeline& pipeline, const VkShaderStageFlags shaderStageFlags, const uint32_t offset,
	                                  const uint32_t size, const void* values) const
	{
		vkCmdPushConstants(handle, pipeline.GetLayout(), shaderStageFlags, offset, size, values);
	}

	void CommandBuffer::Draw(const uint32_t vertexCount, const uint32_t instanceCount, const uint32_t firstVertex, const uint32_t firstInstance) const
	{
		vkCmdDraw(handle, vertexCount, instanceCount, firstVertex, firstInstance);
	}

	void CommandBuffer::DrawIndexed(const uint32_t indexCount, const uint32_t instanceCount, const uint32_t firstIndex,
		const int32_t vertexOffset, const uint32_t firstInstance) const
	{
		vkCmdDrawIndexed(handle, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
	}

	void CommandBuffer::CopyBufferToImage(const Buffer& srcBuffer, const Texture& dstTexture, const std::span<VkBufferImageCopy> regions) const
	{
		vkCmdCopyBufferToImage(handle, srcBuffer.GetNativeHandle(), dstTexture.GetNativeHandle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, regions.size(), regions.data());
	}

	void CommandBuffer::CopyBufferToImageSimple(const Buffer& srcBuffer, const Texture& dstTexture) const
	{
		const auto imageExtent = dstTexture.GetExtent();
		const VkBufferImageCopy imgCopy
		{
			.bufferOffset = 0,
			.bufferRowLength = 0,
			.bufferImageHeight = 0,
			.imageSubresource =
			{
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.mipLevel = 0,
				.baseArrayLayer = 0,
				.layerCount = 1
			},
			.imageExtent = { imageExtent.width, imageExtent.height, imageExtent.depth }
		};

		std::array region = { imgCopy };
		CopyBufferToImage(srcBuffer, dstTexture, region);
	}

	void CommandBuffer::CopyBufferSimple(const Buffer& srcBuffer, const size_t srcOffset, const Buffer& dstBuffer, const size_t dstOffset, const size_t sizeofData) const
	{
		const VkBufferCopy bufferCopy
		{
			.srcOffset = srcOffset,
			.dstOffset = dstOffset,
			.size = sizeofData
		};

		vkCmdCopyBuffer(handle, srcBuffer.GetNativeHandle(), dstBuffer.GetNativeHandle(), 1, &bufferCopy);
	}

	std::pair<VkAccessFlags, VkAccessFlags>  CommandBuffer::QueryOptimalAccessFlagFromImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout)
	{
		VkAccessFlags srcAccess = VK_ACCESS_NONE;
		VkAccessFlags dstAccess = VK_ACCESS_NONE;

		switch (oldLayout)
		{
		default:
			SY_ASSERT(true, "Unsupported image layout.");
			break;
		case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
			srcAccess = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			break;
		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
			srcAccess = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		case VK_IMAGE_LAYOUT_PREINITIALIZED:
			srcAccess = VK_ACCESS_HOST_WRITE_BIT;
		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
			srcAccess = VK_ACCESS_SHADER_READ_BIT;
		case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
			srcAccess = VK_ACCESS_TRANSFER_READ_BIT;
		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
			srcAccess = VK_ACCESS_TRANSFER_WRITE_BIT;
		}

		switch (newLayout)
		{
		default:
			SY_ASSERT(true, "Unsupported image layout.");
			break;
		case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
			dstAccess = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			break;
		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
			dstAccess = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			break;
		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
			if (srcAccess == VK_ACCESS_NONE)
			{
				srcAccess = VK_ACCESS_HOST_READ_BIT | VK_ACCESS_HOST_WRITE_BIT;
			}
			dstAccess = VK_ACCESS_SHADER_READ_BIT;
			break;
		case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
			dstAccess = VK_ACCESS_TRANSFER_READ_BIT;
			break;
		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
			dstAccess = VK_ACCESS_TRANSFER_WRITE_BIT;
			break;
		}

		return { srcAccess, dstAccess };
	}
}
