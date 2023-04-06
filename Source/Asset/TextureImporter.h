#pragma once
#include <PCH.h>
#include <Asset/TextureImportConfig.h>

namespace sy
{
class RawImage;
}

namespace sy::vk
{
class VulkanContext;
}

namespace sy::asset
{
class Texture;
class TextureImporter : public NonCopyable
{
public:
    TextureImporter(vk::VulkanContext& vulkanContext, const fs::path& path, TextureImportConfig config);
    ~TextureImporter();

    void Import();

private:
    void LoadRawImageFromFile();
    void CreateKtxTextureFromRawImage();
    void SetBaseMipToKtxTexture();
    void GenerateMips();
    void ReadbackGeneratedMipsToBuffer();
    void SetGeneratedMipsToKtxTextureFromReadbackBuffers();
    void CompressKtxTexture();
    void ExportKtxTextureToFile();
    void CreateTextureAsset();
    void ExportTextureAssetToFile();

private:
    bool bImported = false;
    vk::VulkanContext& vulkanContext;
    const fs::path& targetPath;
    const std::string targetPathStr;
    const TextureImportConfig config;

    std::unique_ptr<RawImage> rawImage;
    KTXTexture2UniquePtr ktxTextureFromRawImage;
    std::vector<std::unique_ptr<vk::Texture>> generatedMips;
    std::vector<std::unique_ptr<vk::Buffer>> generatedMipReadbackBuffers;
    std::unique_ptr<Texture> newTexture;
};
} // namespace sy::asset