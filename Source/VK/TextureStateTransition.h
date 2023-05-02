#pragma once
#include <PCH.h>
#include <VK/Synchronization.h>

namespace sy::vk
{
class VulkanContext;
class VulkanRHI;
class Texture;
class TextureStateTransition
{
public:
    explicit TextureStateTransition(const VulkanContext& vulkanContext);
    ~TextureStateTransition() = default;

    void SetHandle(const Handle<Texture> targetTextureHandle)
    {
        this->handle = targetTextureHandle;
        this->nativeHandle = std::nullopt;
    }

	void SetNativeHandle(const VkImage image)
	{
        this->handle = {};
        this->nativeHandle = image;
	}

	void SetTexture(const Texture& texture);

	bool IsUsedNativeHandle() const { return nativeHandle != std::nullopt; }

    void SetSourceState(const ETextureState state) { this->srcAccessPattern = QueryAccessPattern(state); }
    void SetDestinationState(const ETextureState state) { this->dstAccessPattern= QueryAccessPattern(state); }
    void SetSourceQueueType(const EQueueType srcQueueType) { this->srcQueueType = srcQueueType; }
    void SetDestinationQueueType(const EQueueType dstQueueType) { this->dstQueueType = dstQueueType; }
    void SetSubresourceRange(const VkImageSubresourceRange range) { this->subresourceRange = range; }

	void OverlapSourceState(const ETextureState state) { srcAccessPattern->Overlap(QueryAccessPattern(state)); }
    void OverlapDestinationState(const ETextureState state) { dstAccessPattern->Overlap(QueryAccessPattern(state)); }

    VkImageMemoryBarrier2 Build() const;

private:
    const VulkanContext& vulkanContext;
    Handle<Texture> handle = {};
    std::optional<VkImage> nativeHandle = std::nullopt;
    std::optional<AccessPattern> srcAccessPattern = std::nullopt;
    std::optional<AccessPattern> dstAccessPattern = std::nullopt;
    std::optional<EQueueType> srcQueueType = std::nullopt;
    std::optional<EQueueType> dstQueueType = std::nullopt;
    std::optional<VkImageSubresourceRange> subresourceRange = std::nullopt;
};
} // namespace sy::vk
