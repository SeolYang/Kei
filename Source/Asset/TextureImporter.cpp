#include <PCH.h>
#include <ktx.h>
#include <Asset/TextureImporter.h>
#include <Asset/TextureAsset.h>
#include <Core/RawImage.h>
#include <VK/VulkanContext.h>
#include <VK/VulkanRHI.h>
#include <VK/Texture.h>
#include <VK/TextureBuilder.h>
#include <VK/Buffer.h>
#include <VK/BufferBuilder.h>
#include <VK/CommandPoolManager.h>
#include <VK/CommandPool.h>
#include <VK/CommandBuffer.h>
#include <VK/MipmapGenerator.h>

namespace sy::asset
{
TextureImporter::TextureImporter(vk::VulkanContext& vulkanContext, const fs::path& path, const TextureImportConfig config) :
    vulkanContext(vulkanContext),
    targetPath(path),
    targetPathStr(path.string()),
    config(config),
    rawImage(std::make_unique<RawImage>())
{
}

TextureImporter::~TextureImporter()
{
}

void TextureImporter::Import()
{
    LoadRawImageFromFile();
    CreateKtxTextureFromRawImage();
    SetBaseMipToKtxTexture();
    GenerateMips();
    ReadbackGeneratedMipsToBuffer();
    SetGeneratedMipsToKtxTextureFromReadbackBuffers();
    CompressKtxTexture();
    ExportKtxTextureToFile();
    CreateTextureAsset();
    ExportTextureAssetToFile();
}

void TextureImporter::LoadRawImageFromFile()
{
    SY_ASSERT(rawImage->LoadFromFile(targetPath), "Failed load raw image from {}.", targetPathStr);
}

void TextureImporter::CreateKtxTextureFromRawImage()
{
    const auto imageExtent = rawImage->GetExtent();
    ktxTextureCreateInfo createInfo{
        .glInternalformat = 0,
        .vkFormat = static_cast<uint32_t>(rawImage->GetEstimatedFormat()),
        .baseWidth = imageExtent.width,
        .baseHeight = imageExtent.height,
        .baseDepth = imageExtent.depth,
        .numDimensions = 2,
        .numLevels = config.IsGenerateMipsWhenImport() ? CalculateMaximumMipCountFromExtent(rawImage->GetExtent()) : 1,
        .numLayers = 1,
        .numFaces = 1,
        .isArray = KTX_FALSE,
        .generateMipmaps = KTX_FALSE};

    ktxTexture2* acquiredKtxTexture = nullptr;
    ktxResult result = ktxTexture2_Create(&createInfo,
                                          KTX_TEXTURE_CREATE_ALLOC_STORAGE,
                                          &acquiredKtxTexture);

	SY_ASSERT(result == KTX_SUCCESS, "Failed to create ktx texture. Error : {}", magic_enum::enum_name(result));
    ktxTextureFromRawImage = KTXTexture2UniquePtr(acquiredKtxTexture, [](ktxTexture2* ptr) {
        ktxTexture_Destroy(ktxTexture(ptr));
    });
}

void TextureImporter::SetBaseMipToKtxTexture()
{
    const auto rawImageDataSpan = rawImage->GetDataSpan();
    const auto result = ktxTexture_SetImageFromMemory(ktxTexture(ktxTextureFromRawImage.get()),
                                                      0, 0, 0,
                                                      rawImageDataSpan.data(),
                                                      static_cast<uint32_t>(rawImageDataSpan.size()));
    SY_ASSERT(result == KTX_SUCCESS, "Failed to set base mip to ktx texture.");
}

void TextureImporter::GenerateMips()
{
    const bool bValidTexture = rawImage != nullptr;
    const bool bGenerateMipsEnabled = config.IsGenerateMipsWhenImport();
    if (bValidTexture && bGenerateMipsEnabled)
    {
        const vk::MipmapGenerator generator(vulkanContext, *rawImage);
        generatedMips = generator.Generate();
    }
}

void TextureImporter::ReadbackGeneratedMipsToBuffer()
{
    generatedMipReadbackBuffers.reserve(generatedMips.size());

    auto& vulkanRHI = vulkanContext.GetRHI();
    auto& cmdPoolManager = vulkanContext.GetCommandPoolManager();
    auto& cmdPool = cmdPoolManager.RequestCommandPool(vk::EQueueType::Graphics);
    for (const auto& generatedMip : generatedMips)
    {
        const auto extent = generatedMip->GetExtent();
        const size_t textureSizeBytes = ImageBlobBytesSize(extent.width, extent.height, rawImage->GetNumChannels(), rawImage->GetBytesPerChannel());

        vk::BufferBuilder readbackBuilder{vulkanContext};
        auto readback = readbackBuilder
                            .SetMemoryUsage(VMA_MEMORY_USAGE_GPU_TO_CPU)
                            .SetMemoryProperty(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
                            .SetSize(textureSizeBytes)
                            .SetUsage(VK_BUFFER_USAGE_TRANSFER_DST_BIT)
                            .Build();

        const auto cmdBuffer = cmdPool.RequestCommandBuffer("Mip Transfer Command Buffer");
        cmdBuffer->Begin();
        cmdBuffer->CopyImageToBuffer(*generatedMip, *readback);
        cmdBuffer->End();
        vulkanRHI.SubmitImmediateTo(*cmdBuffer);

        generatedMipReadbackBuffers.emplace_back(std::move(readback));
    }
}

void TextureImporter::SetGeneratedMipsToKtxTextureFromReadbackBuffers()
{
    auto& vulkanRHI = vulkanContext.GetRHI();
    uint32_t mipLevel = 1;
    for (const auto& readbackBuffer : generatedMipReadbackBuffers)
    {
        const uint8_t* mappedBuffer = reinterpret_cast<const uint8_t*>(vulkanRHI.Map(*readbackBuffer));
        const auto result = ktxTexture_SetImageFromMemory(ktxTexture(ktxTextureFromRawImage.get()),
                                                          mipLevel, 0, 0,
                                                          mappedBuffer,
                                                          static_cast<uint32_t>(readbackBuffer->GetAlignedSize()));
        if (result != KTX_SUCCESS)
        {
            spdlog::warn("Failed to set mip texture {} to ktx texture from memory. Error: {}", mipLevel, magic_enum::enum_name<ktx_error_code_e>(result));
        }

        vulkanRHI.Unmap(*readbackBuffer);
        ++mipLevel;
    }
}

void TextureImporter::CompressKtxTexture()
{
    ktxBasisParams basisParams = {0};
    basisParams.structSize = sizeof(ktxBasisParams);
    basisParams.compressionLevel = TextureCompressionQualityToLevel(config.GetTargetCompressionQuality());
    basisParams.qualityLevel = TextureQualityToLevel(config.GetTargetQuality());
    basisParams.threadCount = std::thread::hardware_concurrency();
    basisParams.uastc = KTX_FALSE;

    const auto result = ktxTexture2_CompressBasisEx(ktxTextureFromRawImage.get(), &basisParams);
    SY_ASSERT(result == KTX_SUCCESS, "Failed to compress texture. Error: {}", magic_enum::enum_name<ktx_error_code_e>(result));
}

void TextureImporter::ExportKtxTextureToFile()
{
    fs::path ktxOutputPath = targetPath;
    ktxOutputPath.replace_extension("ktx");
    ktxTexture_WriteToNamedFile(ktxTexture(ktxTextureFromRawImage.get()), ktxOutputPath.string().c_str());
}

void TextureImporter::CreateTextureAsset()
{
    if (rawImage != nullptr)
    {
        const auto extent = rawImage->GetExtent();
        newTexture = std::make_unique<Texture>(targetPath);
        newTexture->SetExtent(Extent2D{extent.width, extent.height});
        newTexture->SetFormat(rawImage->GetEstimatedFormat());
        newTexture->SetCompressionMode(config.GetTargetCompressionMode());
        newTexture->SetCompressQuality(config.GetTargetCompressionQuality());
        newTexture->SetQuality(config.GetTargetQuality());
    }
}

void TextureImporter::ExportTextureAssetToFile()
{
    if (newTexture != nullptr)
    {
        SaveJsonToFile(newTexture->GetPath(), newTexture->Serialize());
    }
}

} // namespace sy::asset