#pragma once
#include <PCH.h>
#include <VK/VulkanWrapper.h>

namespace sy::vk
{
class CommandPoolAllocator;
class FrameTracker;
class TextureBuilder;
class Texture : public VulkanWrapper<VkImage>
{
public:
    using SubresourceRange = VkImageSubresourceRange;
	using Barrier = VkImageMemoryBarrier2;
	using State = ETextureState;

public:
    explicit Texture(const TextureBuilder& builder);
    ~Texture() override = default;

    [[nodiscard]] auto GetImageType() const { return type; }
    [[nodiscard]] auto GetUsage() const { return usage; }
    [[nodiscard]] auto GetFormat() const { return format; }
    [[nodiscard]] auto GetMemoryUsage() const { return memoryUsage; }
    [[nodiscard]] auto GetMemoryProperty() const { return memoryProperty; }
    [[nodiscard]] auto GetExtent() const { return extent; }
    [[nodiscard]] auto GetArrayLayers() const { return layers; }
    [[nodiscard]] auto GetSamples() const { return samples; }
    [[nodiscard]] auto GetTiling() const { return tiling; }
    [[nodiscard]] auto GetMipLevels() const { return mips; }
    [[nodiscard]] auto IsTextureArray() const { return GetImageType() == VK_IMAGE_TYPE_2D && GetExtent().depth > 1; }
    [[nodiscard]] auto GetNumSubresources() const { return IsTextureArray() ? GetExtent().depth * GetMipLevels() : GetMipLevels(); }
    [[nodiscard]] auto GetImageAspect() const { return FormatToImageAspect(format); }
    [[nodiscard]] auto GetInitialState() const { return initialState; }
    [[nodiscard]] auto GetFullSubresourceRange() const { return SubresourceRange{GetImageAspect(), 0, GetMipLevels(), 0, GetArrayLayers()}; }
    [[nodiscard]] VmaAllocation GetAllocation() const { return allocation; }

private:
    VmaAllocation allocation = VK_NULL_HANDLE;
    const VkImageType type;
    const VkImageUsageFlags usage;
    const VkFormat format;
    const VmaMemoryUsage memoryUsage;
    const VkMemoryPropertyFlags memoryProperty;
    const Extent3D<uint32_t> extent;
    const uint32_t layers;
    const VkSampleCountFlagBits samples;
    const VkImageTiling tiling;
    const State initialState;
    const uint32_t mips;
};
} // namespace sy::vk
