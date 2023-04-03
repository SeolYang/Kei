#pragma once
#include <PCH.h>
#include <Asset/TextureAssetEnums.h>

namespace sy::vk
{
class VulkanContext;
}

namespace sy::asset
{
struct TextureImportConfig
{
    /** Generate full pyramid mips from original texture. */
    const bool                       bGenerateMipsWhenImport  = false;
    const ETextureCompressionMode    TargetCompressionMode    = ETextureCompressionMode::BC7;
    const ETextureCompressionQuality TargetCompressionQuality = ETextureCompressionQuality::Medium;
    const ETextureQuality            TargetQuality            = ETextureQuality::Medium;
};


class TextureImporter
{
public:
    static bool Import2D(vk::VulkanContext& vulkanContext, const fs::path& path, TextureImportConfig config);

private:
    TextureImporter()  = default;
    ~TextureImporter() = default;
};
} // namespace sy::asset