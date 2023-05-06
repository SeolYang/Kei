#include <PCH.h>
#include <VK/Buffer.h>
#include <VK/BufferBuilder.h>
#include <VK/CommandBuffer.h>
#include <VK/CommandPool.h>
#include <VK/CommandPoolAllocator.h>
#include <VK/Fence.h>
#include <VK/FrameTracker.h>
#include <VK/VulkanContext.h>
#include <VK/VulkanRHI.h>
#include <VK/ResourceStateTransition.h>

namespace sy::vk
{
size_t CalculateAlignedBufferSize(VulkanContext& vulkanContext,
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

Buffer::Buffer(const BufferBuilder& builder) :
    VulkanWrapper(builder.name, builder.vulkanContext, VK_OBJECT_TYPE_BUFFER), alignedSize(CalculateAlignedBufferSize(builder.vulkanContext, builder.size, *builder.usage)), usage(*builder.usage | (builder.dataToTransfer.has_value() ? VK_BUFFER_USAGE_TRANSFER_DST_BIT : 0)), memoryUsage(*builder.memoryUsage), initialState(builder.targetInitialState)
{
    const VkBufferCreateInfo createInfo{
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .size = this->alignedSize,
        .usage = this->usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE};

    const VmaAllocationCreateInfo allocationCreateInfo{
        .usage = this->memoryUsage};

    auto& vulkanContext = builder.vulkanContext;
    const auto& vulkanRHI = vulkanContext.GetRHI();

    NativeHandle handle = VK_NULL_HANDLE;
    VK_ASSERT(
        vmaCreateBuffer(vulkanRHI.GetAllocator(), &createInfo, &allocationCreateInfo, &handle, &allocation, nullptr),
        "Failed to create buffer {}.", builder.name);

    UpdateHandle(
        handle,
        [handle, allocation = allocation](const VulkanRHI& rhi) {
            vmaDestroyBuffer(rhi.GetAllocator(), handle, allocation);
        });

    const bool bRequiredDataTransfer = builder.dataToTransfer.has_value();
    const bool bRequiredStateChange = initialState != EBufferState::None;
    if (bRequiredDataTransfer || bRequiredStateChange)
    {
        auto& cmdPoolAllocator = vulkanContext.GetCommandPoolAllocator();
        auto& cmdPool = cmdPoolAllocator.RequestCommandPool(EQueueType::Graphics);
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
                ResourceStateTransition transition{vulkanContext};
                transition.UseBuffer(*this);
                transition.SetSourceState(EBufferState::None);
                transition.SetDestinationState(initialState);
                cmdBuffer->ApplyStateTransition(transition);
            }
        }
        cmdBuffer->End();

        vulkanRHI.SubmitImmediateTo(*cmdBuffer);
    }
}
} // namespace sy::vk
