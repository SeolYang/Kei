#include <PCH.h>
#include <Asset/TextureImportConfig.h>
#include <Asset/Constants.h>

namespace sy::asset
{
json TextureImportConfig::Serialize() const
{
    namespace key = constants::metadata::key;

    json root                         = AssetImportConfig::Serialize();
    root[key::GenerateMipsWhenImport] = bGenerateMipsWhenImport;
    root[key::CompressionMode]        = magic_enum::enum_name(targetCompressionMode);
    root[key::CompressionQuality]     = magic_enum::enum_name(targetCompressionQuality);
    root[key::Quality]                = magic_enum::enum_name(targetQuality);

    return root;
}

void TextureImportConfig::Deserialize(const json& root)
{
    namespace key = constants::metadata::key;

    AssetImportConfig::Deserialize(root);
    bGenerateMipsWhenImport  = ResolveValueFromJson(root, key::GenerateMipsWhenImport, false);
    targetCompressionMode    = ResolveEnumFromJson(root, key::CompressionMode, ETextureCompressionMode::BC7);
    targetCompressionQuality = ResolveEnumFromJson(root, key::CompressionQuality, ETextureCompressionQuality::Medium);
    targetQuality            = ResolveEnumFromJson(root, key::Quality, ETextureQuality::Medium);
}
}