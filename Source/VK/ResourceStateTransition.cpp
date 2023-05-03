#include <PCH.h>
#include <VK/ResourceStateTransition.h>
#include <VK/Texture.h>
#include <VK/Buffer.h>
#include <VK/VulkanContext.h>
#include <VK/VulkanRHI.h>

namespace sy::vk
{
Texture::Barrier ResourceStateTransition<Texture>::Build() const
{
    CheckValidation();

    const VulkanRHI& vulkanRHI = vulkanContext.GetRHI();
    const AccessPattern srcAccessPattern = this->srcAccessPattern ? *this->srcAccessPattern : QueryAccessPattern(ETextureState::None);
    const AccessPattern dstAccessPattern = this->dstAccessPattern ? *this->dstAccessPattern : QueryAccessPattern(ETextureState::None);
    const VkImageMemoryBarrier2 barrier{
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
        .pNext = nullptr,
        .srcStageMask = srcAccessPattern.PipelineStage,
        .srcAccessMask = srcAccessPattern.Access,
        .dstStageMask = dstAccessPattern.PipelineStage,
        .dstAccessMask = dstAccessPattern.Access,
        .oldLayout = srcAccessPattern.ImageLayout,
        .newLayout = dstAccessPattern.ImageLayout,
        .srcQueueFamilyIndex = srcQueueType ? vulkanRHI.GetQueueFamilyIndex(*srcQueueType) : VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = dstQueueType ? vulkanRHI.GetQueueFamilyIndex(*dstQueueType) : VK_QUEUE_FAMILY_IGNORED,
        .image = IsUsedNativeHandle() ? *nativeHandle : handle->GetNative(),
        .subresourceRange = subresourceRange ? *subresourceRange : handle->GetFullSubresourceRange()};

    return barrier;
}

Buffer::Barrier ResourceStateTransition<Buffer>::Build() const
{
    CheckValidation();

    const VulkanRHI& vulkanRHI = vulkanContext.GetRHI();
    const AccessPattern srcAccessPattern = this->srcAccessPattern ? *this->srcAccessPattern : QueryAccessPattern(ETextureState::None);
    const AccessPattern dstAccessPattern = this->dstAccessPattern ? *this->dstAccessPattern : QueryAccessPattern(ETextureState::None);
    const Range<uint32_t> subresourceRange = this->subresourceRange ? *this->subresourceRange : handle->GetFullSubresourceRange();
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
}