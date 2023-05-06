#include <PCH.h>
#include <VK/ResourceStateTransition.h>
#include <VK/Texture.h>
#include <VK/Buffer.h>
#include <VK/VulkanContext.h>
#include <VK/VulkanRHI.h>

namespace sy::vk
{
void ResourceStateTransition::UseResource(const Texture& texture)
{
    Reset();
    handleVariant = texture.GetNative();
    SetSubresourceRange(texture.GetFullSubresourceRange());
}

void ResourceStateTransition::UseResource(const Handle<Texture> handle)
{
    UseResource(*handle);
}

void ResourceStateTransition::UseResource(const Buffer& buffer)
{
    Reset();
    handleVariant = buffer.GetNative();
    SetSubresourceRange(buffer.GetFullSubresourceRange());
}

void ResourceStateTransition::UseResource(const Handle<Buffer> handle)
{
    UseResource(*handle);
}

void ResourceStateTransition::SetSourceState(const ETextureState state)
{
    CheckIsTextureTransition();
    SetSourceAccessPattern(QueryAccessPattern(state));
}

void ResourceStateTransition::SetSourceState(const EBufferState state)
{
    CheckIsBufferTransition();
    SetSourceAccessPattern(QueryAccessPattern(state));
}

void ResourceStateTransition::SetDestinationState(const ETextureState state)
{
    CheckIsTextureTransition();
    SetDestinationAccessPattern(QueryAccessPattern(state));
}

void ResourceStateTransition::SetDestinationState(const EBufferState state)
{
    CheckIsBufferTransition();
    SetDestinationAccessPattern(QueryAccessPattern(state));
}

void ResourceStateTransition::OverlapSourceState(const ETextureState state)
{
    CheckIsTextureTransition();
    OverlapSourceAccessPattern(QueryAccessPattern(state));
}

void ResourceStateTransition::OverlapSourceState(const EBufferState state)
{
    CheckIsBufferTransition();
    OverlapSourceAccessPattern(QueryAccessPattern(state));
}

void ResourceStateTransition::OverlapDestinationState(const ETextureState state)
{
    CheckIsTextureTransition();
    OverlapDestinationAccessPattern(QueryAccessPattern(state));
}

void ResourceStateTransition::OverlapDestinationState(const EBufferState state)
{
    CheckIsBufferTransition();
    OverlapDestinationAccessPattern(QueryAccessPattern(state));
}

void ResourceStateTransition::OverlapSourceAccessPattern(const AccessPattern accessPattern)
{
    if (srcAccessPattern)
    {
        srcAccessPattern->Overlap(accessPattern);
    }
    else
    {
        SetSourceAccessPattern(accessPattern);
    }
}

void ResourceStateTransition::OverlapDestinationAccessPattern(const AccessPattern accessPattern)
{
    if (dstAccessPattern)
    {
        dstAccessPattern->Overlap(accessPattern);
    }
    else
    {
        SetDestinationAccessPattern(accessPattern);
    }
}

void ResourceStateTransition::SetSubresourceRange(const VkImageSubresourceRange range, const bool bUseAsFullRange)
{
    subresourceRangeVariant = range;
    if (bUseAsFullRange)
    {
        fullSubresourceRangeVariant = range;
    }
}

void ResourceStateTransition::SetSubresourceRange(const Range<uint32_t> range, const bool bUseAsFullRange)
{
    subresourceRangeVariant = range;
    if (bUseAsFullRange)
    {
        fullSubresourceRangeVariant = range;
    }
}

VkImageMemoryBarrier2 ResourceStateTransition::AsTextureMemoryBarrier() const
{
    CheckValidation();
    CheckIsTextureTransition();
    CheckTextureSubresourceRangeValidation();
    const VulkanRHI& vulkanRHI = vulkanContext.GetRHI();
    const AccessPattern srcAccessPattern = this->srcAccessPattern ? *this->srcAccessPattern : QueryAccessPattern(ETextureState::None);
    const AccessPattern dstAccessPattern = this->dstAccessPattern ? *this->dstAccessPattern : QueryAccessPattern(ETextureState::None);
    return VkImageMemoryBarrier2{
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
        .image = std::get<VkImage>(handleVariant),
        .subresourceRange = std::get<Texture::SubresourceRange>(subresourceRangeVariant)};
}

VkBufferMemoryBarrier2 ResourceStateTransition::AsBufferMemoryBarrier() const
{
    CheckValidation();
    CheckIsBufferTransition();
    CheckBufferSubresourceRangeValidation();
    const VulkanRHI& vulkanRHI = vulkanContext.GetRHI();
    const AccessPattern srcAccessPattern = this->srcAccessPattern ? *this->srcAccessPattern : QueryAccessPattern(ETextureState::None);
    const AccessPattern dstAccessPattern = this->dstAccessPattern ? *this->dstAccessPattern : QueryAccessPattern(ETextureState::None);
    const auto& subresourceRange = std::get<Range<uint32_t>>(subresourceRangeVariant);
    return VkBufferMemoryBarrier2{
        .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2,
        .pNext = nullptr,
        .srcStageMask = srcAccessPattern.PipelineStage,
        .srcAccessMask = srcAccessPattern.Access,
        .dstStageMask = dstAccessPattern.PipelineStage,
        .dstAccessMask = dstAccessPattern.Access,
        .srcQueueFamilyIndex = srcQueueType ? vulkanRHI.GetQueueFamilyIndex(*srcQueueType) : VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = dstQueueType ? vulkanRHI.GetQueueFamilyIndex(*dstQueueType) : VK_QUEUE_FAMILY_IGNORED,
        .buffer = std::get<VkBuffer>(handleVariant),
        .offset = subresourceRange.Offset,
        .size = subresourceRange.Size};
}
} // namespace sy::vk