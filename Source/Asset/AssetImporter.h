#pragma once
#include <PCH.h>
#include <Asset/Constants.h>
#include <Asset/TextureImporter.h>
#include <Asset/ModelImporter.h>

namespace sy::vk
{
class VulkanContext;
}

namespace sy::asset
{
enum class EAssetType : uint16_t
{
    Texture,
    Model
};

static std::optional<EAssetType> FileExtensionToAssetType(std::string_view extension)
{
    static const robin_hood::unordered_map<std::string, EAssetType> extAssetMap{
        {constants::ext::PNG, EAssetType::Texture},
        {constants::ext::JPG, EAssetType::Texture},
        {constants::ext::JPEG, EAssetType::Texture},

        {constants::ext::GLTF, EAssetType::Model},
        {constants::ext::OBJ, EAssetType::Model},
        {constants::ext::FBX, EAssetType::Model},
    };

    auto normalizedExtension = NormalizeExtension(std::string(extension));
    const auto found = extAssetMap.find(std::string(normalizedExtension));
    return found != extAssetMap.end() ? std::optional<EAssetType>{found->second} : std::nullopt;
}

class AssetImportConfig;
class AssetImporter
{
private:
    struct ImportTarget
    {
        std::string Path;
        EAssetType AssetType;
    };

public:
    AssetImporter(vk::VulkanContext& vulkanContext, bool bForceResetOnExecute);
    ~AssetImporter() = default;

    void Execute();

private:
    void LoadAssetImportConifgsFromFile();

    void ForceResetOnExecute();

    void ImportAssetsFromRootAssetDirectory();
    void ExtractRegularFilesFromRootAssetDirectory();
    void ExtractImportTargetsFromRegularFiles();
    void UpdateUnseenImportTargetsToConfigMap();
    static json GetSerializedDefaultConfig(EAssetType assetType);
    void FilteringReadyToImportTargets();
    void ImportTargetAssets();
    void ImportAsset(const ImportTarget& importTarget, json& config);
    void ImportTextureAsset(const fs::path& path, const json& serializedConfig);
    void ImportModelAsset(const fs::path& path, const json& serializedConfig);
    static TextureImportConfig DeserializeTextureImportConfig(const json& serializedConfig);
    static ModelImportConfig DeserializeModelImportConfig(const json& serializedConfig);
    void FinalizeAssetImport(json& config);

    void ExportAssetImportConfigs();

private:
    vk::VulkanContext& vulkanContext;
    fs::path root;
    const bool bForceResetOnExecute = false;
    std::vector<fs::path> regularFiles;
    std::vector<ImportTarget> importTargets;
    json serializedImportConfigMap;
};
} // namespace sy::asset