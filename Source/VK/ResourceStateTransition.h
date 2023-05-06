#pragma once
#include <PCH.h>
#include <VK/Synchronization.h>

namespace sy::vk
{
class VulkanContext;
class VulkanRHI;
class Texture;
class Buffer;
class ResourceStateTransition
{
public:
    explicit ResourceStateTransition(const VulkanContext& vulkanContext) :
        vulkanContext(vulkanContext)
    {
    }

    ~ResourceStateTransition() = default;

    void Reset()
    {
        handleVariant = {};
        subresourceRangeVariant = {};
        fullSubresourceRangeVariant = {};
        srcAccessPattern = std::nullopt;
        dstAccessPattern = std::nullopt;
        srcQueueType = std::nullopt;
        dstQueueType = std::nullopt;
    }

    /**
	* @warn	This method always reset before setup new texture handle.
	*/
    void UseResource(const Texture& texture);

    /**
	* @warn	This method always reset before setup new texture handle.
	*/
    void UseResource(Handle<Texture> handle);

    /**
	* @warn	This method always reset before setup new texture handle.
	*/
    void UseResource(const VkImage nativeHandle)
    {
        Reset();
        handleVariant = nativeHandle;
    }

    /**
	* @warn	This method always reset before setup new buffer handle.
	*/
    void UseResource(const Buffer& buffer);

    /**
	* @warn	This method always reset before setup new buffer handle.
	*/
    void UseResource(Handle<Buffer> handle);

    /**
	* @warn	This method always reset before setup new buffer handle.
	*/
    void UseResource(const VkBuffer nativeHandle)
    {
        Reset();
        handleVariant = nativeHandle;
    }

    void SetSourceState(ETextureState state);
    void SetSourceState(EBufferState state);
    void SetDestinationState(ETextureState state);
    void SetDestinationState(EBufferState state);

    void OverlapSourceState(ETextureState state);
    void OverlapSourceState(EBufferState state);
    void OverlapDestinationState(ETextureState state);
    void OverlapDestinationState(EBufferState state);

    void SetSourceAccessPattern(const AccessPattern accessPattern) { srcAccessPattern = accessPattern;}
    void SetDestinationAccessPattern(const AccessPattern accessPattern) { dstAccessPattern = accessPattern; }

    void OverlapSourceAccessPattern(AccessPattern accessPattern);
    void OverlapDestinationAccessPattern(AccessPattern accessPattern);

    void SetSourceQueueType(const EQueueType srcQueueType) { this->srcQueueType = srcQueueType; }
    void SetDestinationQueueType(const EQueueType dstQueueType) { this->dstQueueType = dstQueueType; }

    void SetSubresourceRange(VkImageSubresourceRange range, bool bUseAsFullRange = true);
    void SetSubresourceRange(Range<uint32_t> range, bool bUseAsFullRange = true);

    [[nodiscard]] VkImageMemoryBarrier2 AsTextureMemoryBarrier() const;
    [[nodiscard]] VkBufferMemoryBarrier2 AsBufferMemoryBarrier() const;

    [[nodiscard]] bool IsTextureStateTransition() const { return std::holds_alternative<VkImage>(handleVariant); }
    [[nodiscard]] bool IsBufferStateTransition() const { return std::holds_alternative<VkBuffer>(handleVariant); }

private:
    void CheckValidation() const
    {
        const bool bValidStates = srcAccessPattern && dstAccessPattern;
        SY_ASSERT(bValidStates, "Both states are does not setup. It result in redundant transition.");
        const bool bStatesAreNotEqual =
            (srcAccessPattern->Access != dstAccessPattern->Access) ||
            (srcAccessPattern->ImageLayout != dstAccessPattern->ImageLayout) ||
            (srcAccessPattern->PipelineStage != dstAccessPattern->PipelineStage);
        SY_ASSERT(bStatesAreNotEqual, "State are equal. It may result in redudant transition.");

        const bool bHasHandle = handleVariant.index() != 0;
        SY_ASSERT(bHasHandle, "Native Handle does not exist.");
        if (bHasHandle)
        {
            const bool bIsValidNativeHandle = std::holds_alternative<VkImage>(handleVariant) ?
                std::get<VkImage>(handleVariant) != VK_NULL_HANDLE :
                std::get<VkBuffer>(handleVariant) != VK_NULL_HANDLE;
            SY_ASSERT(bIsValidNativeHandle, "Invalid Vulkan Native Handle.");
        }

        SY_ASSERT(subresourceRangeVariant.index() != 0, "Subresource Range does not specified.");
    }

    void CheckIsTextureTransition() const
    {
        SY_ASSERT(std::holds_alternative<VkImage>(handleVariant), "Handle is not texture handle.");
    }

    void CheckTextureSubresourceRangeValidation() const
    {
        SY_ASSERT(std::holds_alternative<VkImageSubresourceRange>(fullSubresourceRangeVariant), "Texture full subresource range does not setup.");
        const VkImageSubresourceRange fullSubresourceRange = std::get<VkImageSubresourceRange>(fullSubresourceRangeVariant);

        SY_ASSERT(std::holds_alternative<VkImageSubresourceRange>(subresourceRangeVariant), "Texture subresource range does not setup.");
        const VkImageSubresourceRange subresourceRange = std::get<VkImageSubresourceRange>(subresourceRangeVariant);
        SY_ASSERT(fullSubresourceRange.aspectMask == subresourceRange.aspectMask, "Aspect mask does not match.");
        SY_ASSERT(subresourceRange.baseMipLevel < fullSubresourceRange.levelCount, "Out of level count.");
        SY_ASSERT(subresourceRange.baseArrayLayer < fullSubresourceRange.layerCount, "Out of layer count.");
    }

    void CheckIsBufferTransition() const
    {
        SY_ASSERT(std::holds_alternative<VkBuffer>(handleVariant), "Handle is not buffer handle.");
    }

    void CheckBufferSubresourceRangeValidation() const
    {
        SY_ASSERT(std::holds_alternative<Range<uint32_t>>(fullSubresourceRangeVariant), "Buffer full subresource range does not setup.");
        const Range<uint32_t> fullSubresourceRange = std::get<Range<uint32_t>>(fullSubresourceRangeVariant);

        SY_ASSERT(std::holds_alternative<Range<uint32_t>>(subresourceRangeVariant), "Buffer subresource range does not setup.");
        const Range<uint32_t> subresourceRange = std::get<Range<uint32_t>>(subresourceRangeVariant);
        SY_ASSERT(fullSubresourceRange.Include(subresourceRange), "Out of subresource range.");
    }

private:
    const VulkanContext& vulkanContext;
    std::variant<std::monostate, VkImage, VkBuffer> handleVariant;
    std::variant<std::monostate, VkImageSubresourceRange, Range<uint32_t>> subresourceRangeVariant = {};
    std::variant<std::monostate, VkImageSubresourceRange, Range<uint32_t>> fullSubresourceRangeVariant = {};
    std::optional<AccessPattern> srcAccessPattern = std::nullopt;
    std::optional<AccessPattern> dstAccessPattern = std::nullopt;
    std::optional<EQueueType> srcQueueType = std::nullopt;
    std::optional<EQueueType> dstQueueType = std::nullopt;
};
} // namespace sy::vk