#include <PCH.h>
#include <ktx.h>
#include <Asset/TextureImporter.h>
#include <Asset/TextureAsset.h>
#include <VK/VulkanContext.h>
#include <VK/VulkanRHI.h>
#include <VK/Texture.h>
#include <VK/TextureBuilder.h>
#include <VK/Buffer.h>
#include <VK/BufferBuilder.h>
#include <VK/CommandPoolManager.h>
#include <VK/CommandPool.h>
#include <VK/CommandBuffer.h>

namespace sy::asset
{
VkFormat FindProperFormatForTextures(const size_t channels, const size_t bytesPerChannel)
{
    const size_t bitsPerChannel = bytesPerChannel * 8;
    if (channels == 1)
    {
        if (bitsPerChannel == 8)
        {
            return VK_FORMAT_R8_UNORM;
        }
        else if (bitsPerChannel == 16)
        {
            return VK_FORMAT_R16_UNORM;
        }
    }
    else if (channels == 2)
    {
        if (bitsPerChannel == 8)
        {
            return VK_FORMAT_R8G8_UNORM;
        }
        else if (bitsPerChannel == 16)
        {
            return VK_FORMAT_R16G16_UNORM;
        }
    }
    else if (channels == 3)
    {
        if (bitsPerChannel == 8)
        {
            return VK_FORMAT_R8G8B8_UNORM;
        }
        else if (bitsPerChannel == 16)
        {
            return VK_FORMAT_R16G16B16_UNORM;
        }
    }
    else if (channels == 4)
    {
        if (bitsPerChannel == 8)
        {
            return VK_FORMAT_R8G8B8A8_UNORM;
        }
        else if (bitsPerChannel == 16)
        {
            return VK_FORMAT_R16G16B16A16_UNORM;
        }
    }

    return VK_FORMAT_UNDEFINED;
}

bool TextureImporter::Import2D(vk::VulkanContext& vulkanContext, const fs::path& path, const TextureImportConfig config)
{
    const std::string pathStr = path.string();
    if (!fs::exists(path))
    {
        spdlog::error("Texture asset {} does not exist.", pathStr);
        return false;
    }

    int width    = 0;
    int height   = 0;
    int channels = 0;

    static constexpr auto StbiDeleter = [](uint8_t* ptr) {
        stbi_image_free(ptr);
    };
    std::unique_ptr<uint8_t, std::function<void(uint8_t*)>> image;

    size_t     bytesPerChannel = 1;
    const bool bIs16BitsImage  = stbi_is_16_bit(pathStr.c_str());
    const bool bIsHDR          = stbi_is_hdr(pathStr.c_str());
    if (bIs16BitsImage)
    {
        image = std::unique_ptr<uint8_t, std::function<void(uint8_t*)>>(
            reinterpret_cast<uint8_t*>(stbi_load_16(pathStr.c_str(), &width, &height, &channels, 0)),
            StbiDeleter);
    }
    else if (bIsHDR)
    {
        spdlog::error("Not supported format. {}", pathStr);
        return false;
    }
    else
    {
        image = std::unique_ptr<uint8_t, std::function<void(uint8_t*)>>(
            reinterpret_cast<uint8_t*>(stbi_load(pathStr.c_str(), &width, &height, &channels, 0)),
            StbiDeleter);
    }

    if (image == nullptr)
    {
        return false;
    }

    const uint32_t mipCounts = CalculateMaximumMipCountFromExtent(Extent3D<int>{width, height, 1});
    const VkFormat format    = FindProperFormatForTextures(channels, bytesPerChannel);
    /** #todo Extend to more generalized import process */
    std::unique_ptr<ktxTexture2, std::function<void(ktxTexture2*)>> newKtxTexture;
    ktxTextureCreateInfo                                            ktxCreateInfo;
    ktxCreateInfo.glInternalformat = 0;
    ktxCreateInfo.vkFormat         = format;
    ktxCreateInfo.baseWidth        = width;
    ktxCreateInfo.baseHeight       = height;
    ktxCreateInfo.baseDepth        = 1;
    ktxCreateInfo.isArray          = KTX_FALSE;
    ktxCreateInfo.generateMipmaps  = false;
    ktxCreateInfo.numLevels        = config.IsGenerateMipsWhenImport() ? mipCounts : 1;
    ktxCreateInfo.numLayers        = 1;
    ktxCreateInfo.numFaces         = 1;
    ktxCreateInfo.numDimensions    = 2;

    const uint32_t compressionLevel = TextureCompressionQualityToLevel(config.GetTargetCompressionQuality());
    const uint32_t qualityLevel     = TextureQualityToLevel(config.GetTargetQuality());
    ktxResult      result           = {};
    {
        ktxTexture2* acquiredKtxTexture = nullptr;

        result = ktxTexture2_Create(&ktxCreateInfo,
                                    KTX_TEXTURE_CREATE_ALLOC_STORAGE,
                                    &acquiredKtxTexture);

        if (result != KTX_SUCCESS)
        {
            spdlog::error("Failed t o create ktx texture. Error: {}", magic_enum::enum_name<ktx_error_code_e>(result));
            return false;
        }

        newKtxTexture = std::unique_ptr<ktxTexture2, std::function<void(ktxTexture2*)>>(acquiredKtxTexture, [](ktxTexture2* ptr) {
            ktxTexture_Destroy(ktxTexture(ptr));
        });
    }

    const size_t blobSize = ImageBlobBytesSize(width, height, channels, bytesPerChannel);
    result                = ktxTexture_SetImageFromMemory(ktxTexture(newKtxTexture.get()), 0, 0, 0, image.get(),
                                                          blobSize);
    if (result != KTX_SUCCESS)
    {
        spdlog::error("Failed to set iamge to ktx texture from memory. Error: {}", magic_enum::enum_name<ktx_error_code_e>(result));
        return false;
    }

    if (config.IsGenerateMipsWhenImport())
    {
        auto& vulkanRHI     = vulkanContext.GetRHI();

        if (vulkanRHI.IsFormatSupportFeatures(format,
                                              VK_FORMAT_FEATURE_2_BLIT_SRC_BIT | VK_FORMAT_FEATURE_2_BLIT_DST_BIT))
        {
            std::vector<std::unique_ptr<vk::Texture>> generatedMips;
            generatedMips.reserve(mipCounts);
            const Extent3D<uint32_t> baseExtent{width, height, 1};

            for (uint32_t mip = 0; mip < mipCounts; ++mip)
            {
                vk::TextureBuilder builder{vulkanContext};
                builder.SetExtent(CalculateMipExtent(baseExtent, mip))
                    .SetType(VK_IMAGE_TYPE_2D)
                    .SetFormat(format)
                    .SetUsage(VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT)
                    .SetMemoryUsage(VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE)
                    .SetTiling(VK_IMAGE_TILING_OPTIMAL)
                    .SetTargetInitialState(vk::ETextureState::TransferRead);

                if (mip == 0)
                {
                    builder.SetDataToTransfer(std::span{reinterpret_cast<const uint8_t*>(image.get()), blobSize});
                }

                generatedMips.emplace_back(builder.Build());
            }

            auto& cmdPoolManager = vulkanContext.GetCommandPoolManager();
            auto& cmdPool        = cmdPoolManager.RequestCommandPool(vk::EQueueType::Graphics);
            {
                const auto cmdBuffer = cmdPool.RequestCommandBuffer("Mip Transfer Command Buffer");
                /** Blit based mipmap generation. */
                cmdBuffer->Begin();
                for (uint32_t mip = 1; mip < mipCounts; ++mip)
                {
                    const auto& currentMip = *generatedMips[mip];
                    const auto& prevMip    = *generatedMips[mip - 1];

                    VkImageBlit imageBlit;
                    ZeroMemory(&imageBlit, sizeof(VkImageBlit));

                    const auto prevMipExtent            = CalculateMipExtent(baseExtent, mip - 1);
                    imageBlit.srcSubresource.aspectMask = vk::FormatToImageAspect(format);
                    imageBlit.srcSubresource.layerCount = 1;
                    imageBlit.srcSubresource.mipLevel   = 0;
                    imageBlit.srcOffsets[1].x           = prevMipExtent.width;
                    imageBlit.srcOffsets[1].y           = prevMipExtent.height;
                    imageBlit.srcOffsets[1].z           = prevMipExtent.depth;

                    const auto mipExtent                = CalculateMipExtent(baseExtent, mip);
                    imageBlit.dstSubresource.aspectMask = imageBlit.srcSubresource.aspectMask;
                    imageBlit.dstSubresource.layerCount = 1;
                    imageBlit.dstSubresource.mipLevel   = 0;
                    imageBlit.dstOffsets[1].x           = mipExtent.width;
                    imageBlit.dstOffsets[1].y           = mipExtent.height;
                    imageBlit.dstOffsets[1].z           = mipExtent.depth;

                    cmdBuffer->ChangeTextureState(
                        vk::ETextureState::TransferRead,
                        vk::ETextureState::TransferWrite,
                        currentMip);
                    cmdBuffer->BlitTexture(prevMip, currentMip, imageBlit);
                    cmdBuffer->ChangeTextureState(
                        vk::ETextureState::TransferWrite,
                        vk::ETextureState::TransferRead,
                        currentMip);
                }
                cmdBuffer->End();
                vulkanRHI.SubmitImmediateTo(*cmdBuffer);
            }

            for (uint32_t mip = 1; mip < mipCounts; ++mip)
            {
                const auto&  mipTexture       = *generatedMips[mip];
                const auto   extent           = mipTexture.GetExtent();
                const size_t textureSizeBytes = ImageBlobBytesSize(extent.width, extent.height, channels, bytesPerChannel);

                vk::BufferBuilder readbackBuilder{vulkanContext};
                auto              readback = readbackBuilder
                                    .SetMemoryUsage(VMA_MEMORY_USAGE_GPU_TO_CPU)
                                    .SetMemoryProperty(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
                                    .SetSize(textureSizeBytes)
                                    .SetUsage(VK_BUFFER_USAGE_TRANSFER_DST_BIT)
                                    .Build();

                const auto cmdBuffer = cmdPool.RequestCommandBuffer("Mip Transfer Command Buffer");
                cmdBuffer->Begin();
                cmdBuffer->CopyImageToBuffer(*generatedMips[mip], *readback);
                cmdBuffer->End();
                vulkanRHI.SubmitImmediateTo(*cmdBuffer);

                const uint8_t* mappedTexture = reinterpret_cast<const uint8_t*>(vulkanRHI.Map(*readback));
                result = ktxTexture_SetImageFromMemory(ktxTexture(newKtxTexture.get()),
                                                       mip, 0, 0,
                                                       mappedTexture,
                                                       textureSizeBytes);
                if (result != KTX_SUCCESS)
                {
                    spdlog::warn("Failed to set mip texture {} to ktx texture from memory. Error: {}", mip, magic_enum::enum_name<ktx_error_code_e>(result));
                }

                vulkanRHI.Unmap(*readback);
            }
        }
        else
        {
            SY_ASSERT(false, "Format {} does not support blit features.", magic_enum::enum_name<VkFormat>(format));
        }
    }

    /* Compressio **/
    ktxBasisParams basisParams   = {0};
    basisParams.structSize       = sizeof(ktxBasisParams);
    basisParams.compressionLevel = compressionLevel;
    basisParams.qualityLevel     = qualityLevel;
    basisParams.threadCount      = std::thread::hardware_concurrency();
    basisParams.uastc            = KTX_FALSE;

    result = ktxTexture2_CompressBasisEx(newKtxTexture.get(), &basisParams);
    if (result != KTX_SUCCESS)
    {
        spdlog::error("Failed to compress texture. Error: {}", magic_enum::enum_name<ktx_error_code_e>(result));
        return false;
    }

    fs::path ktxOutputPath = path;
    ktxOutputPath.replace_extension("ktx");
    ktxTexture_WriteToNamedFile(ktxTexture(newKtxTexture.get()), ktxOutputPath.string().c_str());

    auto newTexture = std::make_unique<Texture>(path);
    newTexture->SetExtent(Extent2D<uint32_t>{(uint32_t)width, (uint32_t)height});
    newTexture->SetFormat(format);
    newTexture->SetCompressionMode(config.GetTargetCompressionMode());
    newTexture->SetCompressQuality(config.GetTargetCompressionQuality());
    newTexture->SetQuality(config.GetTargetQuality());

    SaveJsonToFile(newTexture->GetPath(), newTexture->Serialize());
    return true;
}
} // namespace sy::asset