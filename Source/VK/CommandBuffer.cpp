#include <PCH.h>
#include <VK/CommandBuffer.h>
#include <VK/CommandPool.h>
#include <VK/VulkanRHI.h>
#include <VK/Pipeline.h>
#include <VK/Buffer.h>
#include <VK/Texture.h>

#include "ResourceStateTracker.h"

namespace sy
{
	namespace vk
	{
		CommandBuffer::CommandBuffer( std::string_view name, const VulkanRHI& vulkanRHI, const CommandPool& cmdPool ) :
			VulkanWrapper<VkCommandBuffer>(name, vulkanRHI, VK_OBJECT_TYPE_COMMAND_BUFFER,
			                               VK_DESTROY_LAMBDA_SIGNATURE(VkCommandBuffer)
			                               {
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

			Native_t handle = VK_NULL_HANDLE;
			VK_ASSERT(vkAllocateCommandBuffers(vulkanRHI.GetDevice(), &allocInfo, &handle),
			          "Failed to creating command buffer.");
			UpdateHandle(handle);
		}

		void CommandBuffer::Reset() const
		{
			vkResetCommandBuffer(GetNativeHandle(), 0);
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

			VK_ASSERT(vkBeginCommandBuffer(GetNativeHandle(), &beginInfo), "Faeild to begin command buffer {}.",
			          GetName());
		}

		void CommandBuffer::End() const
		{
			VK_ASSERT(vkEndCommandBuffer(GetNativeHandle()), "Failed to end command buffer {}.", GetName());
		}

		void CommandBuffer::BeginRendering( const VkRenderingInfo& renderingInfo ) const
		{
			vkCmdBeginRendering(GetNativeHandle(), &renderingInfo);
		}

		void CommandBuffer::EndRendering() const
		{
			vkCmdEndRendering(GetNativeHandle());
		}

		void CommandBuffer::ChangeState( const EBufferState srcState, const EBufferState dstState,
		                                 const VkBuffer buffer, const size_t offset, const size_t size ) const
		{
			const AccessPattern srcAccess = QueryAccessPattern(srcState);
			const AccessPattern dstAccess = QueryAccessPattern(dstState);
			BufferMemoryBarrier(
			                    srcAccess.PipelineStage, dstAccess.PipelineStage,
			                    srcAccess.Access, dstAccess.Access,
			                    buffer, offset, size);
		}

		void CommandBuffer::ChangeState( const EBufferState srcState, const EBufferState dstState,
		                                 const Buffer& buffer ) const
		{
			ChangeState(srcState, dstState, buffer.GetNativeHandle(), 0, buffer.GetAlignedSize());
		}

		void CommandBuffer::ChangeState( const ETextureState srcState, const ETextureState dstState,
		                                 const VkImage image, const VkImageAspectFlags aspectMask,
		                                 const uint32_t mipLevelCount, const uint32_t baseMipLevel,
		                                 const uint32_t arrayLayerCount, const uint32_t baseArrayLayer ) const
		{
			const AccessPattern srcAccess = QueryAccessPattern(srcState);
			const AccessPattern dstAccess = QueryAccessPattern(dstState);
			ImageMemoryBarrier(
			                   srcAccess.PipelineStage, dstAccess.PipelineStage,
			                   srcAccess.Access, dstAccess.Access,
			                   image,
			                   srcAccess.ImageLayout, dstAccess.ImageLayout,
			                   aspectMask, mipLevelCount, baseMipLevel, arrayLayerCount, baseArrayLayer);
		}

		void CommandBuffer::ChangeState( const ETextureState srcState, const ETextureState dstState,
		                                 const Texture& texture ) const
		{
			ChangeState(srcState, dstState, texture.GetNativeHandle(), FormatToImageAspect(texture.GetFormat()),
			            texture.GetMipLevels(), 0, 1, 0);
		}

		void CommandBuffer::FlushStateTransitions( ResourceStateTracker& resourceStateTracker ) const
		{
			using TextureStateTransition = ResourceStateTracker::TextureStateTransition;
			using BufferStateTransition = ResourceStateTracker::BufferStateTransition;
			const std::vector<TextureStateTransition> textureStateTransitions = resourceStateTracker.
					FlushTextureTransitions();
			std::vector<VkImageMemoryBarrier2> imageBarriers;
			imageBarriers.reserve(textureStateTransitions.size());
			for (const TextureStateTransition& transition : textureStateTransitions)
			{
				const Texture& texture        = transition.Target;
				const AccessPattern srcAccess = QueryAccessPattern(transition.Before);
				const AccessPattern dstAccess = QueryAccessPattern(transition.After);
				imageBarriers.emplace_back(VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2, nullptr,
				                           srcAccess.PipelineStage, srcAccess.Access,
				                           dstAccess.PipelineStage, dstAccess.Access,
				                           srcAccess.ImageLayout, dstAccess.ImageLayout,
				                           VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED,
				                           texture.GetNativeHandle(), VkImageSubresourceRange{
					                           FormatToImageAspect(texture.GetFormat()),
					                           transition.SubResourceRange.MipLevel,
					                           transition.SubResourceRange.MipLevelCount,
					                           transition.SubResourceRange.ArrayLayer,
					                           transition.SubResourceRange.ArrayLayerCount
				                           });
			}

			const std::vector<BufferStateTransition> bufferStateTransitions = resourceStateTracker.
					FlushBufferTransitions();
			std::vector<VkBufferMemoryBarrier2> bufferBarriers;
			bufferBarriers.reserve(bufferStateTransitions.size());
			for (const BufferStateTransition& transition : bufferStateTransitions)
			{
				const Buffer& buffer          = transition.Target;
				const AccessPattern srcAccess = QueryAccessPattern(transition.Before);
				const AccessPattern dstAccess = QueryAccessPattern(transition.After);
				bufferBarriers.emplace_back(VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2,
				                            nullptr,
				                            srcAccess.PipelineStage, srcAccess.Access,
				                            dstAccess.PipelineStage, dstAccess.Access,
				                            VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED,
				                            buffer.GetNativeHandle(), 0, buffer.GetAlignedSize());
			}

			PipelineBarrier({}, bufferBarriers, imageBarriers);
		}

		void CommandBuffer::BindPipeline( const Pipeline& pipeline ) const
		{
			vkCmdBindPipeline(GetNativeHandle(), pipeline.GetBindPoint(), pipeline.GetNativeHandle());
		}

		void CommandBuffer::BindDescriptorSet( VkDescriptorSet descriptorSet, const Pipeline& pipeline ) const
		{
			const VkDescriptorSet descriptorSets[ ] = { descriptorSet, };
			vkCmdBindDescriptorSets(GetNativeHandle(), pipeline.GetBindPoint(), pipeline.GetLayout(), 0, 1,
			                        descriptorSets, 0, nullptr);
		}

		void CommandBuffer::BindVertexBuffers( const uint32_t firstBinding, const std::span<CRef<Buffer>> buffers,
		                                       const std::span<size_t> offsets ) const
		{
			std::vector<VkBuffer> handles;
			handles.resize(buffers.size());
			std::transform(buffers.begin(), buffers.end(),
			               handles.begin(),
			               []( const Buffer& buffer )
			               {
				               return buffer.GetNativeHandle();
			               });

			BindVertexBuffers(firstBinding, handles, offsets);
		}

		void CommandBuffer::BindVertexBuffers( const uint32_t firstBinding, const std::span<VkBuffer> buffers,
		                                       const std::span<size_t> offsets ) const
		{
			vkCmdBindVertexBuffers(GetNativeHandle(), firstBinding, static_cast<uint32_t>(buffers.size()),
			                       buffers.data(), offsets.data());
		}

		void CommandBuffer::BindIndexBuffer( const Buffer& indexBuffer, const size_t offset ) const
		{
			BindIndexBuffer(indexBuffer.GetNativeHandle(), offset);
		}

		void CommandBuffer::BindIndexBuffer( const VkBuffer indexBuffer, const size_t offset ) const
		{
			vkCmdBindIndexBuffer(GetNativeHandle(), indexBuffer, offset, VK_INDEX_TYPE_UINT32);
		}

		void CommandBuffer::PushConstants( const Pipeline& pipeline, const VkShaderStageFlags shaderStageFlags,
		                                   const uint32_t offset,
		                                   const uint32_t size, const void* values ) const
		{
			vkCmdPushConstants(GetNativeHandle(), pipeline.GetLayout(), shaderStageFlags, offset, size, values);
		}

		void CommandBuffer::Draw( const uint32_t vertexCount, const uint32_t instanceCount, const uint32_t firstVertex,
		                          const uint32_t firstInstance ) const
		{
			vkCmdDraw(GetNativeHandle(), vertexCount, instanceCount, firstVertex, firstInstance);
		}

		void CommandBuffer::DrawIndexed( const uint32_t indexCount, const uint32_t instanceCount,
		                                 const uint32_t firstIndex,
		                                 const int32_t vertexOffset, const uint32_t firstInstance ) const
		{
			vkCmdDrawIndexed(GetNativeHandle(), indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
		}

		void CommandBuffer::CopyBufferToImage( const Buffer& srcBuffer, const Texture& dstTexture,
		                                       const std::span<VkBufferImageCopy> regions ) const
		{
			vkCmdCopyBufferToImage(GetNativeHandle(), srcBuffer.GetNativeHandle(), dstTexture.GetNativeHandle(),
			                       VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, static_cast<uint32_t>(regions.size()),
			                       regions.data());
		}

		void CommandBuffer::CopyBufferToImageSimple( const Buffer& srcBuffer, const Texture& dstTexture ) const
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

		void CommandBuffer::CopyBufferSimple( const Buffer& srcBuffer, const size_t srcOffset, const Buffer& dstBuffer,
		                                      const size_t dstOffset, const size_t sizeofData ) const
		{
			const VkBufferCopy bufferCopy
			{
				.srcOffset = srcOffset,
				.dstOffset = dstOffset,
				.size = sizeofData
			};

			vkCmdCopyBuffer(GetNativeHandle(), srcBuffer.GetNativeHandle(), dstBuffer.GetNativeHandle(), 1,
			                &bufferCopy);
		}

		void CommandBuffer::ImageMemoryBarrier( const VkPipelineStageFlags2 srcStage,
		                                        const VkPipelineStageFlags2 dstStage,
		                                        const VkAccessFlags2 srcAccess, const VkAccessFlags2 dstAccess,
		                                        const VkImage image, const VkImageLayout oldLayout,
		                                        const VkImageLayout newLayout, const VkImageAspectFlags aspectMask,
		                                        const uint32_t mipLevelCount, const uint32_t baseMipLevel,
		                                        const uint32_t arrayLayerCount, const uint32_t baseArrayLayer ) const
		{
			const VkImageMemoryBarrier2 barrier
			{
				.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
				.pNext = nullptr,
				.srcStageMask = srcStage,
				.srcAccessMask = srcAccess,
				.dstStageMask = dstStage,
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

			VkImageMemoryBarrier2 barriers[ ] = { barrier };
			PipelineBarrier({}, {}, barriers);
		}

		void CommandBuffer::BufferMemoryBarrier( const VkPipelineStageFlags2 srcStage,
		                                         const VkPipelineStageFlags2 dstStage,
		                                         const VkAccessFlags2 srcAccess, const VkAccessFlags2 dstAccess,
		                                         VkBuffer buffer, const size_t offset, const size_t size ) const
		{
			const VkBufferMemoryBarrier2 barrier
			{
				VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2,
				nullptr,
				srcStage,
				srcAccess,
				dstStage,
				dstAccess,
				VK_QUEUE_FAMILY_IGNORED,
				VK_QUEUE_FAMILY_IGNORED,
				buffer,
				offset,
				size
			};

			VkBufferMemoryBarrier2 barriers[ ] = { barrier };
			PipelineBarrier({}, barriers, {});
		}

		void CommandBuffer::PipelineBarrier( std::span<VkMemoryBarrier2> memoryBarriers,
		                                     std::span<VkBufferMemoryBarrier2> bufferMemoryBarriers,
		                                     std::span<VkImageMemoryBarrier2> imageMemoryBarriers ) const
		{
			const VkDependencyInfo dependencyInfo
			{
				.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
				.pNext = nullptr,
				.memoryBarrierCount = static_cast<uint32_t>(memoryBarriers.size()),
				.pMemoryBarriers = memoryBarriers.data(),
				.bufferMemoryBarrierCount = static_cast<uint32_t>(bufferMemoryBarriers.size()),
				.pBufferMemoryBarriers = bufferMemoryBarriers.data(),
				.imageMemoryBarrierCount = static_cast<uint32_t>(imageMemoryBarriers.size()),
				.pImageMemoryBarriers = imageMemoryBarriers.data()
			};

			vkCmdPipelineBarrier2(GetNativeHandle(), &dependencyInfo);
		}
	}
}
