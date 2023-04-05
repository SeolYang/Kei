#pragma once
#include <PCH.h>
#include <Asset/TextureAssetEnums.h>
#include <Asset/AssetImportConfig.h>

namespace sy::vk
{
class VulkanContext;
}

namespace sy::asset
{
struct TextureImportConfig : public AssetImportConfig
{
    TextureImportConfig& SetGenerateMipsWhenImport(const bool enable)
    {
        bGenerateMipsWhenImport = enable;
        return *this;
    }

    TextureImportConfig& SetTargetCompressionMode(ETextureCompressionMode mode)
    {
        targetCompressionMode = mode;
        return *this;
    }

    TextureImportConfig& SetTargetCompressionQuality(const ETextureCompressionQuality quality)
    {
        targetCompressionQuality = quality;
        return *this;
    }

    TextureImportConfig& SetTargetQuality(const ETextureQuality quality)
    {
        targetQuality = quality;
        return *this;
    }

    [[nodiscard]] auto IsGenerateMipsWhenImport() const { return bGenerateMipsWhenImport; }
    [[nodiscard]] auto GetTargetCompressionMode() const { return targetCompressionMode; }
    [[nodiscard]] auto GetTargetCompressionQuality() const { return targetCompressionQuality; }
    [[nodiscard]] auto GetTargetQuality() const { return targetQuality; }
	
	json Serialize() const override;
    void Deserialize(const json& root) override;

private:
    /** Generate full pyramid mips from original texture. */
    bool                       bGenerateMipsWhenImport  = false;
    ETextureCompressionMode    targetCompressionMode    = ETextureCompressionMode::BC7;
    ETextureCompressionQuality targetCompressionQuality = ETextureCompressionQuality::Medium;
    ETextureQuality            targetQuality            = ETextureQuality::Medium;
};


class TextureImporter
{
public:
    static bool Import2D(vk::VulkanContext& vulkanContext, const fs::path& path, TextureImportConfig config);

private:
    TextureImporter()  = delete;
    ~TextureImporter() = delete;
};
} // namespace sy::asset