#include <PCH.h>
#include <VK/ResourceStateTransition.h>
#include <VK/Texture.h>
#include <VK/Buffer.h>
#include <VK/VulkanContext.h>
#include <VK/VulkanRHI.h>

namespace sy::vk
{
void ResourceStateTransition::UseTexture(const Texture& texture)
{
    Reset();
    handleVariant = texture.GetNative();
    SetSubresourceRange(texture.GetFullSubresourceRange());
}

void ResourceStateTransition::UseTexture(const Handle<Texture> handle)
{
    UseTexture(*handle);
}


void ResourceStateTransition::UseBuffer(const Buffer& buffer)
{
	Reset();
    handleVariant = buffer.GetNative();
    SetSubresourceRange(buffer.GetFullSubresourceRange());
}

void ResourceStateTransition::UseBuffer(const Handle<Buffer> handle)
{
    UseBuffer(*handle);
}

void ResourceStateTransition::SetSourceState(const ETextureState state)
{
    CheckIsTextureTransition();
    srcAccessPattern = QueryAccessPattern(state);
}

void ResourceStateTransition::SetSourceState(const EBufferState state)
{
    CheckIsBufferTransition();
	srcAccessPattern = QueryAccessPattern(state);
}

void ResourceStateTransition::SetDestinationState(const ETextureState state)
{
	CheckIsTextureTransition();
    dstAccessPattern = QueryAccessPattern(state);
}

void ResourceStateTransition::SetDestinationState(const EBufferState state)
{
    CheckIsBufferTransition();
	dstAccessPattern = QueryAccessPattern(state);
}

void ResourceStateTransition::OverlapSourceState(const ETextureState state)
{
	if (srcAccessPattern)
	{
        CheckIsTextureTransition();
        srcAccessPattern->Overlap(QueryAccessPattern(state));
	}
	else
	{
        SetSourceState(state);
	}
}

void ResourceStateTransition::OverlapSourceState(const EBufferState state)
{
    if (srcAccessPattern)
    {
        CheckIsBufferTransition();
        srcAccessPattern->Overlap(QueryAccessPattern(state));
    }
    else
    {
        SetSourceState(state);
    }
}

void ResourceStateTransition::OverlapDestinationState(const ETextureState state)
{
    if (dstAccessPattern)
    {
        CheckIsTextureTransition();
        dstAccessPattern->Overlap(QueryAccessPattern(state));
    }
    else
    {
        SetDestinationState(state);
    }
}

void ResourceStateTransition::OverlapDestinationState(const EBufferState state)
{
    if (srcAccessPattern)
    {
        CheckIsBufferTransition();
        dstAccessPattern->Overlap(QueryAccessPattern(state));
    }
    else
    {
        SetDestinationState(state);
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
}