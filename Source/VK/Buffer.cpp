#include <PCH.h>
#include <VK/Buffer.h>
#include <VK/BufferBuilder.h>
#include <VK/CommandBuffer.h>
#include <VK/CommandPool.h>
#include <VK/CommandPoolManager.h>
#include <VK/Fence.h>
#include <VK/FrameTracker.h>
#include <VK/VulkanContext.h>
#include <VK/VulkanRHI.h>

namespace sy
{
	namespace vk
	{
		size_t CalculateAlignedBufferSize(const VulkanContext& vulkanContext,
			const size_t originSize,
			const VkBufferUsageFlags bufferUsage)
		{
			const auto& vulkanRHI = vulkanContext.GetRHI();
			size_t alignedSize = originSize;
			switch (bufferUsage)
			{
				case VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT:
					alignedSize = vulkanRHI.PadUniformBufferSize(alignedSize);
					break;
				case VK_BUFFER_USAGE_STORAGE_BUFFER_BIT:
					alignedSize = vulkanRHI.PadStorageBufferSize(alignedSize);
					break;
			}

			return alignedSize;
		}

		Buffer::Buffer(const BufferBuilder& builder)
			: VulkanWrapper(builder.name, builder.vulkanContext.GetRHI(),
				VK_OBJECT_TYPE_BUFFER)
			, alignedSize(CalculateAlignedBufferSize(builder.vulkanContext,
				  builder.size, *builder.usage))
			, usage(*builder.usage | (builder.dataToTransfer.has_value() ? VK_BUFFER_USAGE_TRANSFER_DST_BIT : 0))
			, memoryUsage(*builder.memoryUsage)
			, initialState(builder.targetInitialState)
		{
			const VkBufferCreateInfo createInfo{
				.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
				.pNext = nullptr,
				.flags = 0,
				.size = this->alignedSize,
				.usage = this->usage,
				.sharingMode = VK_SHARING_MODE_EXCLUSIVE
			};

			const VmaAllocationCreateInfo allocationCreateInfo{
				.usage = this->memoryUsage
			};

			const auto& vulkanContext = builder.vulkanContext;
			const auto& vulkanRHI = vulkanContext.GetRHI();
			Native_t handle = VK_NULL_HANDLE;
			VK_ASSERT(
				vmaCreateBuffer(vulkanRHI.GetAllocator(), &createInfo,
					&allocationCreateInfo, &handle, &allocation, nullptr),
				"Failed to create buffer {}.", builder.name);
			UpdateHandle(handle);

			const bool bRequiredDataTransfer = builder.dataToTransfer.has_value();
			const bool bRequiredStateChange = initialState != EBufferState::None;
			if (bRequiredDataTransfer || bRequiredStateChange)
			{
				auto& cmdPoolManager = vulkanContext.GetCommandPoolManager();
				auto& cmdPool = cmdPoolManager.RequestCommandPool(EQueueType::Graphics);
				const auto cmdBuffer =
					cmdPool.RequestCommandBuffer("Buffer Transfer Command Buffer");

				std::unique_ptr<Buffer> stagingBuffer = nullptr;
				cmdBuffer->Begin();
				{
					if (bRequiredDataTransfer)
					{
						stagingBuffer =
							BufferBuilder::StagingBufferTemplate(builder.vulkanContext)
								.SetName("Staging Buffer-To Buffer")
								.SetSize(builder.size)
								.Build();

						void* mappedStagingBuffer = vulkanRHI.Map(*stagingBuffer);
						std::memcpy(mappedStagingBuffer, builder.dataToTransfer->data(),
							builder.dataToTransfer->size());
						vulkanRHI.Unmap(*stagingBuffer);

						cmdBuffer->CopyBufferSimple(*stagingBuffer, 0, *this, 0, builder.size);
					}

					if (bRequiredStateChange)
					{
						cmdBuffer->ChangeState(EBufferState::None, initialState, *this);
					}
				}
				cmdBuffer->End();

				vulkanRHI.SubmitImmediateTo(*cmdBuffer);
			}
		}

		Buffer::~Buffer()
		{
			const VulkanRHI& context = GetRHI();
			vmaDestroyBuffer(context.GetAllocator(), GetNativeHandle(), allocation);
		}
	} // namespace vk
} // namespace sy
