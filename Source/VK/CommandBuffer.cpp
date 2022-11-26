#include <Core.h>
#include <VK/CommandBuffer.h>
#include <VK/CommandPool.h>
#include <VK/VulkanInstance.h>

#include "Fence.h"

namespace sy
{
	CommandBuffer::CommandBuffer(std::string_view name, const VulkanInstance& vulkanInstance, const CommandPool& cmdPool) :
		VulkanWrapper<VkCommandBuffer>(name, vulkanInstance, VK_DESTROY_LAMBDA_SIGNATURE(VkCommandBuffer)
	{
		// DO NOTHING
	}),
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

		VK_ASSERT(vkAllocateCommandBuffers(vulkanInstance.GetLogicalDevice(), &allocInfo, &handle), "Failed to creating command buffer.");
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
		SY_ASSERT(srcStage == dstStage, "Redundant image layout change detected.");
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
		VK_ASSERT(vkCmdPipelineBarrier(handle,
			srcStage, dstStage, 0, 
			memoryBarriers.size(), memoryBarriers.data(),
			bufferMemoryBarriers.size(), bufferMemoryBarriers.data(), 
			imageMemoryBarriers.size(), imageMemoryBarriers.data()), "Failed to insert pipeline barrier.");
	}

	std::pair<VkAccessFlags, VkAccessFlags>  CommandBuffer::QueryOptimalAccessFlagFromImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout)
	{
		VkAccessFlags srcAccess = VK_ACCESS_NONE;
		VkAccessFlags dstAccess = VK_ACCESS_NONE;

		switch (oldLayout)
		{
		default:
			SY_ASSERT(true, "Non-support image layout.");
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
			SY_ASSERT(true, "Non-support image layout.");
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

		return {srcAccess, dstAccess};
	}
}
