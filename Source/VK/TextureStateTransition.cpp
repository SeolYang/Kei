#include <PCH.h>
#include <VK/TextureStateTransition.h>
#include <VK/Texture.h>
#include <VK/VulkanContext.h>
#include <VK/VulkanRHI.h>

namespace sy::vk
{
TextureStateTransition::TextureStateTransition(const VulkanContext& vulkanContext) :
    vulkanContext(vulkanContext)
{
}

void TextureStateTransition::SetTexture(const Texture& texture)
{
    SetNativeHandle(texture.GetNative());
    SetSubresourceRange(texture.GetFullSubresourceRange());
}

VkImageMemoryBarrier2 TextureStateTransition::Build() const
{
    const bool bValidStates = this->srcAccessPattern && this->dstAccessPattern;
    SY_ASSERT(bValidStates, "Both states are does not setup. It result in redundant transition.");
    const bool bStatesAreNotEqual =
        (srcAccessPattern->Access != dstAccessPattern->Access) &&
        (srcAccessPattern->ImageLayout != dstAccessPattern->ImageLayout) &&
        (srcAccessPattern->PipelineStage != dstAccessPattern->PipelineStage);
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

    const AccessPattern srcAccessPattern = this->srcAccessPattern ? *this->srcAccessPattern : QueryAccessPattern(ETextureState::None);
    const AccessPattern dstAccessPattern = this->dstAccessPattern ? *this->dstAccessPattern : QueryAccessPattern(ETextureState::None);

    const VulkanRHI& vulkanRHI = vulkanContext.GetRHI();
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


} // namespace sy::vk