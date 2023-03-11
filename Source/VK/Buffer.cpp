#include <PCH.h>
#include <VK/Buffer.h>
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
		Buffer::Buffer(const std::string_view name, const VulkanRHI& vulkanRHI, const BufferInfo info, const EBufferState initialState) :
			VulkanWrapper(name, vulkanRHI, VK_OBJECT_TYPE_BUFFER),
			initialState(initialState)
		{
			size_t alignedBufferSize = info.Size;
			switch (info.UsageFlags)
			{
			case VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT:
				alignedBufferSize = vulkanRHI.PadUniformBufferSize(alignedBufferSize);
				break;

			case VK_BUFFER_USAGE_STORAGE_BUFFER_BIT:
				alignedBufferSize = vulkanRHI.PadStorageBufferSize(alignedBufferSize);
				break;

			default:
				break;
			}

			const VkBufferCreateInfo createInfo
			{
				.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
				.pNext = nullptr,
				.flags = 0,
				.size = alignedBufferSize,
				.usage = info.UsageFlags,
				.sharingMode = VK_SHARING_MODE_EXCLUSIVE
			};

			const VmaAllocationCreateInfo allocationCreateInfo
			{
				.usage = info.MemoryUsage
			};

			Native_t handle = VK_NULL_HANDLE;
			VK_ASSERT(vmaCreateBuffer(vulkanRHI.GetAllocator(), &createInfo, &allocationCreateInfo, &handle, &allocation, nullptr), "Failed to create buffer {}.", name);
			UpdateHandle(handle);
			this->info = { alignedBufferSize, info.UsageFlags, info.MemoryUsage };
		}

		Buffer::~Buffer()
		{
			const VulkanRHI& context = GetContext();
			vmaDestroyBuffer(context.GetAllocator(), GetNativeHandle(), allocation);
		}

		std::unique_ptr<Buffer> CreateBufferWithData(std::string_view name, const VulkanContext& vulkanContext, VkBufferUsageFlags bufferUsage,
			size_t sizeOfData, const void* data, const EBufferState initialState)
		{
			const auto& vulkanRHI = vulkanContext.GetVulkanRHI();
			auto& cmdPoolManager = vulkanContext.GetCommandPoolManager();
			const auto& frameTracker = vulkanContext.GetFrameTracker();
			auto newBuffer = std::make_unique<Buffer>(name, vulkanRHI, BufferInfo{ sizeOfData, bufferUsage | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY }, initialState);
			const auto stagingBuffer = CreateStagingBuffer(std::format("Staging buffer for {}", name), vulkanContext, sizeOfData);
			void* mappedData = vulkanRHI.Map(*stagingBuffer);
			memcpy(mappedData, data, sizeOfData);
			vulkanRHI.Unmap(*stagingBuffer);

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
			vulkanRHI.SubmitTo(*transferCmdBuffer, uploadFence);
			uploadFence.Wait();
			uploadFence.Reset();
			return newBuffer;
		}

		std::unique_ptr<Buffer> CreateStagingBuffer(std::string_view name, const VulkanContext& vulkanContext,
		                                            const size_t bufferSize)
		{
			return std::make_unique<Buffer>(name, vulkanContext.GetVulkanRHI(), BufferInfo{bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY});
		}
	}
}
