#include <Core.h>
#include <VK/Buffer.h>
#include <VK/VulkanContext.h>
#include <VK/CommandBuffer.h>
#include <VK/CommandPool.h>
#include <VK/Fence.h>
#include <CommandPoolManager.h>
#include <FrameTracker.h>

namespace sy
{
	Buffer::Buffer(std::string_view name, const VulkanContext& vulkanContext, const size_t bufferSize, const VkBufferCreateFlags bufferCreateFlags, const VkBufferUsageFlags bufferUsageFlags,
		const VmaMemoryUsage memoryUsage) :
		VulkanWrapper(name, vulkanContext),
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

	std::unique_ptr<Buffer> Buffer::CreateVertexBuffer(CommandPoolManager& cmdPoolManager, const FrameTracker& frameTracker,
		std::string_view name, const VulkanContext& vulkanContext, size_t sizeOfData, void* vertices)
	{
		auto newBuffer = std::make_unique<Buffer>(name, vulkanContext, sizeOfData, 0, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
		const auto stagingBuffer = CreateStagingBuffer(std::format("Staging buffer for {}", name), vulkanContext, sizeOfData);
		void* mappedData = vulkanContext.Map(*stagingBuffer);
		memcpy(mappedData, vertices, sizeOfData);
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
