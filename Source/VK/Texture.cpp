#include <PCH.h>
#include <VK/Texture.h>
#include <VK/TextureBuilder.h>
#include <VK/VulkanRHI.h>
#include <VK/Fence.h>
#include <VK/Buffer.h>
#include <VK/CommandPool.h>
#include <VK/CommandBuffer.h>
#include <VK/CommandPoolAllocator.h>
#include <VK/FrameTracker.h>
#include <VK/VulkanContext.h>

namespace sy::vk
{
Texture::Texture(const TextureBuilder& builder) :
    VulkanWrapper(builder.name, builder.vulkanContext, VK_OBJECT_TYPE_IMAGE),
    type(*builder.type),
    usage(*builder.usage | (builder.dataToTransfer.has_value() ? VK_IMAGE_USAGE_TRANSFER_DST_BIT : 0)),
    format(builder.format),
    memoryUsage(*builder.memoryUsage),
    memoryProperty(builder.memoryProperty),
    extent(*builder.extent),
    layers(builder.layers),
    samples(builder.samples),
    tiling(builder.tiling),
    initialState(builder.targetInitialState),
    mips(builder.mips)
{
    const VkImageCreateInfo imageCreateInfo{
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .imageType = type,
        .format = format,
        .extent = VkExtent3D{
            extent.width,
            type != VK_IMAGE_TYPE_1D ? extent.height : 1,
            type == VK_IMAGE_TYPE_3D ? extent.depth : 1},
        .mipLevels = mips,
        .arrayLayers = layers,
        .samples = samples,
        .tiling = tiling,
        .usage = usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED};

    const VmaAllocationCreateInfo allocationCreateInfo{
        .usage = memoryUsage,
        .requiredFlags = memoryProperty};

    auto& vulkanContext = builder.vulkanContext;
    const auto& vulkanRHI = vulkanContext.GetRHI();
    NativeHandle handle = VK_NULL_HANDLE;
    VK_ASSERT(vmaCreateImage(vulkanRHI.GetAllocator(),
                             &imageCreateInfo, &allocationCreateInfo,
                             &handle, &allocation,
                             nullptr),
              "Failed to create image {}.", builder.name);

    UpdateHandle(
        handle,
        [handle, allocation = allocation](const VulkanRHI& rhi) {
            vmaDestroyImage(rhi.GetAllocator(), handle, allocation);
        });

    const bool bRequiredDataTransfer = builder.dataToTransfer.has_value();
    const bool bRequiredStateTransfer = builder.targetInitialState != ETextureState::None;
    if (bRequiredDataTransfer || bRequiredStateTransfer)
    {
        auto& cmdPoolAllocator = vulkanContext.GetCommandPoolAllocator();
        auto& cmdPool = cmdPoolAllocator.RequestCommandPool(EQueueType::Graphics);
        const auto cmdBuffer = cmdPool.RequestCommandBuffer("Buffer Transfer Command Buffer");

        std::unique_ptr<Buffer> stagingBuffer = nullptr;
        cmdBuffer->Begin();
        {
            auto currentState = ETextureState::None;
            if (bRequiredDataTransfer)
            {
                stagingBuffer = BufferBuilder::StagingBufferTemplate(builder.vulkanContext)
                                    .SetName("Staging Buffer-To Texture")
                                    .SetSize(builder.dataToTransfer->size_bytes())
                                    .Build();

                void* mappedStagingBuffer = vulkanRHI.Map(*stagingBuffer);
                std::memcpy(mappedStagingBuffer, builder.dataToTransfer->data(),
                            builder.dataToTransfer->size());
                vulkanRHI.Unmap(*stagingBuffer);

                cmdBuffer->ChangeTextureState(ETextureState::None, ETextureState::TransferWrite, *this);
                currentState = ETextureState::TransferWrite;

                if (builder.copyInfos.empty())
                {
                    cmdBuffer->CopyBufferToImageSimple(*stagingBuffer, *this);
                }
                else
                {
                    cmdBuffer->CopyBufferToImage(*stagingBuffer, *this, builder.copyInfos);
                }
            }

            cmdBuffer->ChangeTextureState(currentState, initialState, *this);
        }
        cmdBuffer->End();

        vulkanRHI.SubmitImmediateTo(*cmdBuffer);
    }
}
} // namespace sy::vk
