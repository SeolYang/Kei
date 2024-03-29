#pragma once
#include <PCH.h>

namespace sy::vk
{
class VulkanContext;
class Texture;
class TextureBuilder
{
public:
    explicit TextureBuilder(VulkanContext& vulkanContext) :
        vulkanContext(vulkanContext)
    {
    }

    TextureBuilder& SetName(const std::string_view name)
    {
        this->name = name;
        return *this;
    }

    TextureBuilder& SetType(const VkImageType type)
    {
        this->type = type;
        return *this;
    }

    TextureBuilder& SetUsage(const VkImageUsageFlags newUsage)
    {
        usage = newUsage;
        return *this;
    }

	TextureBuilder& AddUsage(const VkImageUsageFlags additionalUsage)
	{
		if (usage == std::nullopt)
		{
            usage = additionalUsage;
		}
		else
		{
            *usage |= additionalUsage;
		}

        return *this;
	}

    TextureBuilder& SetFormat(const VkFormat format)
    {
        this->format = format;
        return *this;
    }

    TextureBuilder& SetMemoryProperty(const VkMemoryPropertyFlags memoryProperty)
    {
        this->memoryProperty = memoryProperty;
        return *this;
    }

    TextureBuilder& SetMemoryUsage(const VmaMemoryUsage memoryUsage)
    {
        this->memoryUsage = memoryUsage;
        return *this;
    }

    TextureBuilder& SetExtent(const Extent3D<uint32_t> extent)
    {
        SY_ASSERT(extent.width > 0 && extent.height > 0 && extent.depth > 0, "Extent must be greater than zero.");
        this->extent = extent;
        return *this;
    }

    TextureBuilder& SetExtent(const Extent2D<uint32_t> extent)
    {
        return SetExtent(Extent3D<uint32_t>{extent.width, extent.height, 1});
    }

    TextureBuilder& SetMips(const uint32_t mips)
    {
        SY_ASSERT(mips > 0, "Texture must have at least one mips.");
        this->mips = mips;
        return *this;
    }

    TextureBuilder& SetArrayLayers(const uint32_t layers)
    {
        SY_ASSERT(layers > 0, "Texture must have at least one layers.");
        this->layers = layers;
        return *this;
    }

    TextureBuilder& SetSampleCount(const VkSampleCountFlagBits samples)
    {
        this->samples = samples;
        return *this;
    }

    TextureBuilder& SetTiling(const VkImageTiling tiling)
    {
        this->tiling = tiling;
        return *this;
    }

    TextureBuilder& SetTargetInitialState(const ETextureState targetInitialState)
    {
        this->targetInitialState = targetInitialState;
        return *this;
    }

    template <typename T>
    TextureBuilder& SetDataToTransfer(const std::span<const T> typedData)
    {
        [[likely]] if (const bool bIsValidData = typedData.size() > 0 || typedData.data() != nullptr;
                       bIsValidData)
        {
            const auto bytesOfData = std::span<const uint8_t>{
                reinterpret_cast<const uint8_t*>(typedData.data()),
                typedData.size_bytes()};
            dataToTransfer = bytesOfData;
        }
        else
        {
            dataToTransfer = std::nullopt;
        }

        return *this;
    }

	TextureBuilder& AddCopyInfo(const VkBufferImageCopy copyInfo)
	{
        copyInfos.push_back(copyInfo);
        return *this;
	}

    std::unique_ptr<Texture> Build() const;

public:
    static TextureBuilder Texture2DShaderResourceTemplate(VulkanContext& vulkanContext);
    static TextureBuilder Texture2DRenderTargetTemplate(VulkanContext& vulkanContext);
    static TextureBuilder Texture2DDepthStencilTemplate(VulkanContext& vulkanContext);

private:
    friend class Texture;
    VulkanContext&                          vulkanContext;
    std::string                             name               = "Texture";
    std::optional<VkImageType>              type               = std::nullopt;
    std::optional<VkImageUsageFlags>        usage              = std::nullopt;
    VkFormat                                format             = VK_FORMAT_UNDEFINED;
    std::optional<VmaMemoryUsage>           memoryUsage        = std::nullopt;
    VkMemoryPropertyFlags                   memoryProperty     = 0;
    std::optional<Extent3D<uint32_t>>       extent             = std::nullopt;
    uint32_t                                mips               = 1;
    uint32_t                                layers             = 1;
    VkSampleCountFlagBits                   samples            = VK_SAMPLE_COUNT_1_BIT;
    VkImageTiling                           tiling             = VK_IMAGE_TILING_OPTIMAL;
    ETextureState                           targetInitialState = ETextureState::None;
    std::optional<std::span<const uint8_t>> dataToTransfer     = std::nullopt;
    std::vector<VkBufferImageCopy>          copyInfos;
};
} // namespace sy::vk
