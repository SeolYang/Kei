#include <PCH.h>
#include <VK/BufferStateTransition.h>
#include <VK/Buffer.h>
#include <VK/VulkanContext.h>
#include <VK/VulkanRHI.h>

namespace sy::vk
{
BufferStateTransition::BufferStateTransition(const VulkanContext& vulkanContext) :
    vulkanContext(vulkanContext)
{
}

void BufferStateTransition::SetBuffer(const Buffer& buffer)
{
    SetNativeHandle(buffer.GetNative());
    SetSubresourceRange(buffer.GetFullSubresourceRange());
}

VkBufferMemoryBarrier2 BufferStateTransition::Build() const
{
    const bool bValidStates = srcState && dstState;
    SY_ASSERT(bValidStates, "Both states are does not setup. It result in redundant transition.");
    const bool bStatesAreNotEqual = bValidStates && (*srcState != *dstState);
    SY_ASSERT(bStatesAreNotEqual, "State are equal. It may result in redudant transition.");

    bool bIsValidNativeHandle = false;
    if (IsUsedNativeHandle())
    {
        bIsValidNativeHandle = nativeHandle != VK_NULL_HANDLE;
        SY_ASSERT(bIsValidNativeHandle, "Invalid Vulkan Native Handle.");

        const bool bHasSubresourceRange = subresourceRange != std::nullopt;
        SY_ASSERT(bHasSubresourceRange, "Native Handle State Transtion must specify subresource range manually.");
    }
    else
    {
        const bool bIsValidHandle = handle.IsValid();
        SY_ASSERT(bIsValidHandle, "Invalid Texture Handle.");
        bIsValidNativeHandle = handle->GetNative() != VK_NULL_HANDLE;
    }
    SY_ASSERT(bIsValidNativeHandle, "Invalid Vulkan Native Handle.");

    const AccessPattern srcAccessPattern = srcState ? QueryAccessPattern(*srcState) : QueryAccessPattern(ETextureState::None);
    const AccessPattern dstAccessPattern = dstState ? QueryAccessPattern(*dstState) : QueryAccessPattern(ETextureState::None);

    const Range<uint32_t> subresourceRange = this->subresourceRange ? *this->subresourceRange : handle->GetFullSubresourceRange();
    const VulkanRHI& vulkanRHI = vulkanContext.GetRHI();
    const VkBufferMemoryBarrier2 barrier{
        .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2,
        .pNext = nullptr,
        .srcStageMask = srcAccessPattern.PipelineStage,
        .srcAccessMask = srcAccessPattern.Access,
        .dstStageMask = dstAccessPattern.PipelineStage,
        .dstAccessMask = dstAccessPattern.Access,
        .srcQueueFamilyIndex = srcQueueType ? vulkanRHI.GetQueueFamilyIndex(*srcQueueType) : VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = dstQueueType ? vulkanRHI.GetQueueFamilyIndex(*dstQueueType) : VK_QUEUE_FAMILY_IGNORED,
        .buffer = IsUsedNativeHandle() ? *nativeHandle : handle->GetNative(),
        .offset = subresourceRange.Offset,
        .size = subresourceRange.Size};

    return barrier;
}
} // namespace sy::vk