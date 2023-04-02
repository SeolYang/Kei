#include <PCH.h>
#include <ktx.h>
#include <Asset/TextureImporter.h>
#include <Asset/TextureAsset.h>

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

bool TextureImporter::Import(const fs::path& path, const TextureImportConfig config)
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

    const VkFormat format = FindProperFormatForTextures(channels, bytesPerChannel);

    /** #todo Extend to more generalized import process */
    std::unique_ptr<ktxTexture2, std::function<void(ktxTexture2*)>> newKtxTexture;
    ktxTextureCreateInfo                                            ktxCreateInfo;
    ktxCreateInfo.glInternalformat = 0;
    ktxCreateInfo.vkFormat         = format;
    ktxCreateInfo.baseWidth        = width;
    ktxCreateInfo.baseHeight       = height;
    ktxCreateInfo.baseDepth        = 1;
    ktxCreateInfo.isArray          = KTX_FALSE;
    ktxCreateInfo.generateMipmaps  = config.bGenerateMips ? KTX_TRUE : KTX_FALSE;
    ktxCreateInfo.numLevels        = 1;
    ktxCreateInfo.numLayers        = 1;
    ktxCreateInfo.numFaces         = 1;
    ktxCreateInfo.numDimensions    = 2;

    const uint32_t compressionLevel = TextureCompressionQualityToLevel(config.TargetCompressionQuality);
    const uint32_t qualityLevel     = TextureQualityToLevel(config.TargetQuality);
    const size_t   srcLevel         = 0;
    const size_t   srcLayer         = 0;
    const size_t   srcFaceSlice     = 0;
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
    result = ktxTexture_SetImageFromMemory(ktxTexture(newKtxTexture.get()), srcLevel, srcLayer, srcFaceSlice, image.get(),
                                           blobSize);
    if (result != KTX_SUCCESS)
    {
        spdlog::error("Failed to set iamge to ktx texture from memory. Error: {}", magic_enum::enum_name<ktx_error_code_e>(result));
        return false;
    }

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
    newTexture->SetCompressionMode(config.TargetCompressionMode);
    newTexture->SetCompressQuality(config.TargetCompressionQuality);
    newTexture->SetQuality(config.TargetQuality);
    SaveJsonToFile(newTexture->GetPath(), newTexture->Serialize());
    return true;
}
} // namespace sy::asset