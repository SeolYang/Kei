#include <Core/Core.h>
#include <VK/Buffer.h>
#include <VK/VulkanContext.h>
#include <VK/CommandBuffer.h>
#include <VK/CommandPool.h>
#include <VK/Fence.h>
#include <VK/CommandPoolManager.h>
#include <VK/FrameTracker.h>

namespace sy
{
	namespace vk
	{
		Buffer::Buffer(std::string_view name, const VulkanContext& vulkanContext, const size_t bufferSize, const VkBufferCreateFlags bufferCreateFlags, const VkBufferUsageFlags bufferUsageFlags,
			const VmaMemoryUsage memoryUsage) :
			VulkanWrapper(name, vulkanContext, VK_OBJECT_TYPE_BUFFER),
			bufferUsageFlags(bufferUsageFlags)
		{
			size_t alignedBufferSize = bufferSize;
			switch (bufferUsageFlags)
			{
			case VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT:
				alignedBufferSize = vulkanContext.PadUniformBufferSize(bufferSize);
				break;

			case VK_BUFFER_USAGE_STORAGE_BUFFER_BIT:
				alignedBufferSize = vulkanContext.PadStorageBufferSize(bufferSize);
				break;

			default:
				break;
			}
			this->bufferSize = alignedBufferSize;

			const VkBufferCreateInfo createInfo
			{
				.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
				.pNext = nullptr,
				.flags = bufferCreateFlags,
				.size = alignedBufferSize,
				.usage = bufferUsageFlags,
				.sharingMode = VK_SHARING_MODE_EXCLUSIVE
			};

			const VmaAllocationCreateInfo allocationCreateInfo
			{
				.usage = memoryUsage
			};

			VK_ASSERT(vmaCreateBuffer(vulkanContext.GetAllocator(), &createInfo, &allocationCreateInfo, &handle, &allocation, nullptr), "Failed to create buffer {}.", name);
		}

		Buffer::~Buffer()
		{
			vmaDestroyBuffer(vulkanContext.GetAllocator(), handle, allocation);
		}

		std::unique_ptr<Buffer> CreateStagingBuffer(std::string_view name, const VulkanContext& vulkanContext,
			const size_t bufferSize)
		{
			return std::make_unique<Buffer>(name, vulkanContext, bufferSize, 0, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
		}

		std::unique_ptr<Buffer> CreateIndexBuffer(CommandPoolManager& cmdPoolManager,
		                                          const FrameTracker& frameTracker, std::string_view name, const VulkanContext& vulkanContext,
		                                          const std::span<const uint32_t> indices)
		{
			return CreateBufferWithData(cmdPoolManager, frameTracker, name, vulkanContext, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, sizeof(uint32_t) * indices.size(), indices.data());
		}

		std::unique_ptr<Buffer> CreateBufferWithData(CommandPoolManager& cmdPoolManager, const FrameTracker& frameTracker,
			std::string_view name, const VulkanContext& vulkanContext, const VkBufferUsageFlags bufferUsage, size_t sizeOfData, const void* data)
		{
			auto newBuffer = std::make_unique<Buffer>(name, vulkanContext, sizeOfData, 0, bufferUsage | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
			const auto stagingBuffer = CreateStagingBuffer(std::format("Staging buffer for {}", name), vulkanContext, sizeOfData);
			void* mappedData = vulkanContext.Map(*stagingBuffer);
			memcpy(mappedData, data, sizeOfData);
			vulkanContext.Unmap(*stagingBuffer);

			// @todo: Batched upload data
			// @todo: Use of Transfer Queue ; need more practicing and knowledge about pipeline barriers!
			auto& transferCmdPool = cmdPoolManager.RequestCommandPool(EQueueType::Transfer);
			const auto transferCmdBuffer = transferCmdPool.RequestCommandBuffer(std::format("Vertex buffer {} transfer cmd buffer", name));
			transferCmdBuffer->Begin();
			{
				transferCmdBuffer->CopyBufferSimple(*stagingBuffer, 0, *newBuffer, 0, sizeOfData);
			}
			transferCmdBuffer->End();

			const auto& uploadFence = frameTracker.GetCurrentInFlightUploadFence();
			vulkanContext.SubmitTo(*transferCmdBuffer, uploadFence);
			uploadFence.Wait();
			uploadFence.Reset();
			return newBuffer;
		}
	}
}
