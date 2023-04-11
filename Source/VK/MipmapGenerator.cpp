#include <PCH.h>
#include <VK/MipmapGenerator.h>
#include <VK/VulkanContext.h>
#include <VK/VulkanRHI.h>
#include <VK/CommandPoolAllocator.h>
#include <VK/CommandPool.h>
#include <VK/CommandBuffer.h>
#include <VK/TextureBuilder.h>
#include <VK/Texture.h>
#include <Core/RawImage.h>

namespace sy::vk
{
MipmapGenerator::MipmapGenerator(VulkanContext& vulkanContext, const RawImage& rawImage) :
    vulkanContext(vulkanContext),
    rawImage(rawImage)
{
}

bool MipmapGenerator::IsFormatSupportBlit(const VkFormat format, vk::VulkanRHI& rhi)
{
    return rhi.IsFormatSupportFeatures(format, VK_FORMAT_FEATURE_2_BLIT_SRC_BIT | VK_FORMAT_FEATURE_2_BLIT_DST_BIT);
}

std::vector<std::unique_ptr<sy::vk::Texture>> MipmapGenerator::Generate() const
{
    std::vector<std::unique_ptr<vk::Texture>> generatedMips = CreateMipTextures();
    SY_ASSERT(!generatedMips.empty(), "Nothing created from raw image.");
    GenerateMips(generatedMips);
    generatedMips.erase(generatedMips.begin());
    return generatedMips;
}

std::vector<std::unique_ptr<sy::vk::Texture>> MipmapGenerator::CreateMipTextures() const
{
    std::vector<std::unique_ptr<vk::Texture>> generatedMips;
    const size_t mipCount = CalculateMaximumMipCountFromExtent(rawImage.GetExtent());
    generatedMips.reserve(mipCount);
    for (uint32_t mip = 0; mip < mipCount; ++mip)
    {
        vk::TextureBuilder builder{vulkanContext};
        builder.SetExtent(CalculateMipExtent(rawImage.GetExtent(), mip))
            .SetType(VK_IMAGE_TYPE_2D)
            .SetFormat(rawImage.GetEstimatedFormat())
            .SetUsage(VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT)
            .SetMemoryUsage(VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE)
            .SetTiling(VK_IMAGE_TILING_OPTIMAL)
            .SetTargetInitialState(vk::ETextureState::TransferRead);

        if (mip == 0)
        {
            builder.SetDataToTransfer(rawImage.GetDataSpan());
        }

        generatedMips.emplace_back(builder.Build());
    }

    return generatedMips;
}

void MipmapGenerator::GenerateMips(const std::vector<std::unique_ptr<vk::Texture>>& textures) const
{
    auto& vulkanRHI = vulkanContext.GetRHI();
    auto& cmdPoolAllocator = vulkanContext.GetCommandPoolAllocator();
    auto& cmdPool = cmdPoolAllocator.RequestCommandPool(vk::EQueueType::Graphics);
    const auto cmdBuffer = cmdPool.RequestCommandBuffer("Mip Transfer Command Buffer");

    cmdBuffer->Begin();
    for (uint32_t mip = 1; mip < textures.size(); ++mip)
    {
        SubmitBlitToCommandBuffer(*cmdBuffer, *textures[mip - 1], *textures[mip]);
    }
    cmdBuffer->End();

    vulkanRHI.SubmitImmediateTo(*cmdBuffer);
}

void MipmapGenerator::SubmitBlitToCommandBuffer(const vk::CommandBuffer& cmdBuffer, const vk::Texture& srcMip, const vk::Texture& dstMip) const
{
    VkImageBlit imageBlit;
    ZeroMemory(&imageBlit, sizeof(VkImageBlit));

    const auto srcExtent = srcMip.GetExtent();
    imageBlit.srcSubresource.aspectMask = vk::FormatToImageAspect(rawImage.GetEstimatedFormat());
    imageBlit.srcSubresource.layerCount = 1;
    imageBlit.srcSubresource.mipLevel = 0;
    imageBlit.srcOffsets[1].x = srcExtent.width;
    imageBlit.srcOffsets[1].y = srcExtent.height;
    imageBlit.srcOffsets[1].z = srcExtent.depth;

    const auto dstExtent = dstMip.GetExtent();
    imageBlit.dstSubresource.aspectMask = imageBlit.srcSubresource.aspectMask;
    imageBlit.dstSubresource.layerCount = 1;
    imageBlit.dstSubresource.mipLevel = 0;
    imageBlit.dstOffsets[1].x = dstExtent.width;
    imageBlit.dstOffsets[1].y = dstExtent.height;
    imageBlit.dstOffsets[1].z = dstExtent.depth;

	TextureStateTransition stateTransition{vulkanContext};
    stateTransition.SetTexture(dstMip);
    stateTransition.SetSourceState(vk::ETextureState::TransferRead);
	stateTransition.SetDestinationState(vk::ETextureState::TransferWrite);
    cmdBuffer.ApplyStateTransition(stateTransition);

    cmdBuffer.BlitTexture(srcMip, dstMip, imageBlit);

    stateTransition.SetSourceState(vk::ETextureState::TransferWrite);
    stateTransition.SetDestinationState(vk::ETextureState::TransferRead);
    cmdBuffer.ApplyStateTransition(stateTransition);
}
} // namespace sy::vk