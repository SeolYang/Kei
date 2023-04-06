#pragma once
#include <PCH.h>

namespace sy
{
class RawImage;
}

namespace sy::vk
{
class VulkanContext;
class VulkanRHI;
class CommandBuffer;
class Texture;
class MipmapGenerator : public NonCopyable
{
private:
    static bool IsFormatSupportBlit(const VkFormat format, vk::VulkanRHI& rhi);

public:
    MipmapGenerator(VulkanContext& vulkanContext, const RawImage& rawImage);

    std::vector<std::unique_ptr<vk::Texture>> Generate() const;

private:
    std::vector<std::unique_ptr<vk::Texture>> CreateMipTextures() const;
    void GenerateMips(const std::vector<std::unique_ptr<vk::Texture>>& textures) const;
    void SubmitBlitToCommandBuffer(const vk::CommandBuffer& cmdBuffer, const vk::Texture& srcMip, const vk::Texture& dstMip) const;

private:
    VulkanContext& vulkanContext;
    const RawImage& rawImage;
};
} // namespace sy::vk
