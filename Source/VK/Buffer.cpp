#include <PCH.h>
#include <VK/Buffer.h>
#include <VK/BufferBuilder.h>
#include <VK/VulkanContext.h>
#include <VK/VulkanRHI.h>
#include <VK/CommandBuffer.h>
#include <VK/CommandPool.h>
#include <VK/Fence.h>
#include <VK/CommandPoolManager.h>
#include <VK/FrameTracker.h>

namespace sy
{
	namespace vk
	{
		size_t CalculateAlignedBufferSize(const VulkanContext& vulkanContext, const size_t originSize, const VkBufferUsageFlags bufferUsage)
		{
			const auto& vulkanRHI = vulkanContext.GetVulkanRHI();
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

		Buffer::Buffer(const BufferBuilder& builder) :
			VulkanWrapper(builder.name, builder.vulkanContext.GetVulkanRHI(), VK_OBJECT_TYPE_BUFFER),
			alignedSize(CalculateAlignedBufferSize(builder.vulkanContext, builder.size, *builder.usage)),
			usage(*builder.usage | (builder.dataToTransfer.has_value() ? VK_BUFFER_USAGE_TRANSFER_DST_BIT : 0)),
			memoryUsage(*builder.memoryUsage),
			initialState(builder.targetInitialState)
		{
			const VkBufferCreateInfo createInfo
			{
				.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
				.pNext = nullptr,
				.flags = 0,
				.size = this->alignedSize,
				.usage = this->usage,
				.sharingMode = VK_SHARING_MODE_EXCLUSIVE
			};

			const VmaAllocationCreateInfo allocationCreateInfo
			{
				.usage = this->memoryUsage
			};

			const auto& vulkanContext = builder.vulkanContext;
			const auto& vulkanRHI = vulkanContext.GetVulkanRHI();
			Native_t handle = VK_NULL_HANDLE;
			VK_ASSERT(vmaCreateBuffer(vulkanRHI.GetAllocator(), &createInfo, &allocationCreateInfo, &handle, &allocation, nullptr), "Failed to create buffer {}.", builder.name);
			UpdateHandle(handle);

			const bool bRequiredStateTransfer = initialState != EBufferState::None;
			const bool bRequiredDataTransfer = builder.dataToTransfer.has_value();
			if (bRequiredStateTransfer || bRequiredDataTransfer)
			{
				auto& cmdPoolManager = vulkanContext.GetCommandPoolManager();
				auto& cmdPool = cmdPoolManager.RequestCommandPool(EQueueType::Graphics);
				const auto cmdBuffer = cmdPool.RequestCommandBuffer("Buffer Transfer Command Buffer");

				cmdBuffer->Begin();
				{
					if (bRequiredDataTransfer)
					{
						BufferBuilder stagingBufferBuilder{ builder.vulkanContext };
						stagingBufferBuilder.SetName("Staging Buffer")
							.SetUsage(VK_BUFFER_USAGE_TRANSFER_SRC_BIT)
							.SetSize(builder.size)
							.SetMemoryUsage(VMA_MEMORY_USAGE_CPU_ONLY);
						const auto stagingBuffer = stagingBufferBuilder.Build();
						void* mappedStagingBuffer = vulkanRHI.Map(*stagingBuffer);
						std::memcpy(mappedStagingBuffer, builder.dataToTransfer->data(), builder.dataToTransfer->size());
						vulkanRHI.Unmap(*stagingBuffer);

						cmdBuffer->CopyBufferSimple(*stagingBuffer, 0, *this, 0, builder.size);
					}

					if (bRequiredStateTransfer)
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
			const VulkanRHI& context = GetContext();
			vmaDestroyBuffer(context.GetAllocator(), GetNativeHandle(), allocation);
		}
	}
}
