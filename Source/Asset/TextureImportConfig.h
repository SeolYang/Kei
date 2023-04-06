#pragma once
#include <Asset/AssetImportConfig.h>
#include <Asset/TextureAssetEnums.h>

namespace sy::asset
{
class TextureImportConfig : public AssetImportConfig
{
public:
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

}
