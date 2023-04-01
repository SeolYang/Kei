#include <PCH.h>
#include <encoder/basisu_enc.h>
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

    uint8_t* image = nullptr;

    size_t     bytesPerChannel = 1;
    const bool bIs16BitsImage  = stbi_is_16_bit(pathStr.c_str());
    const bool bIsHDR          = stbi_is_hdr(pathStr.c_str());
    if (bIs16BitsImage)
    {
        image           = reinterpret_cast<uint8_t*>(stbi_load_16(pathStr.c_str(), &width, &height, &channels, 0));
        bytesPerChannel = 2;
    }
    else if (bIsHDR)
    {
        spdlog::error("Not supported format. {}", pathStr);
        return false;
    }
    else
    {
        image = stbi_load(pathStr.c_str(), &width, &height, &channels, 0);
    }

    if (image == nullptr)
    {
        return false;
    }

    VkFormat format     = FindProperFormatForTextures(channels, bytesPerChannel);
    auto     newTexture = std::make_unique<Texture>(path);
    newTexture->SetExtent(Extent2D<uint32_t>{(uint32_t)width, (uint32_t)height});
    newTexture->SetFormat(format);
    //newTexture->SetCompressQuality()
    //newTexture->SetCompressionMode()
    //newTexture->SetSampler(co)

    SaveJsonToFile(newTexture->GetPath(), newTexture->Serialize());

    const size_t blobSize = ImageBlobBytesSize(width, height, channels, bytesPerChannel);
    SaveBlobToFile(newTexture->GetBlobPath(), std::span{image, blobSize});

    stbi_image_free(image);
    return true;
}
} // namespace sy::asset