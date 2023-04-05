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

    if (extension[0] == '.')
    {
        extension = extension.substr(1);
    }

    // #todo to_lowercase
    const auto found = extAssetMap.find(std::string(extension));
    return found != extAssetMap.end() ? std::optional<EAssetType>{found->second} : std::nullopt;
}

class AssetImporter
{
public:
    AssetImporter(vk::VulkanContext& vulkanContext, bool bForceResetOnExecute);
    ~AssetImporter() = default;

    void Execute();

private:
    void LoadAssetImportConifgsFromFile();

    void ForceResetOnExecute();

    void                       ImportAssetsFromRootAssetDirectory();
    void                       ExtractRegularFilesInRootAssetDirectory();
    void                       ExtractImportTargetsFromRegularFiles();
    void                       UpdateUnseenImportTargetsToConfigMap();
    static json                GetSerializedDefaultConfig(EAssetType assetType);
    void                       FilteringImportTargets();
    void                       ImportTargetAssets();
    void                       ImportAsset(const std::string_view path, const EAssetType assetType, json& config);
    void                       ImportTextureAsset(const fs::path& path, const json& serializedConfig);
    void                       ImportModelAsset(const fs::path& path, const json& serializedConfig);
    static TextureImportConfig DeserializeTextureImportConfig(const json& serializedConfig);
    static ModelImportConfig   DeserializeModelImportConfig(const json& serializedConfig);
    void                       FinalizeAssetImport(json& config);

    void ExportAssetImportConfigs();

private:
    vk::VulkanContext&                              vulkanContext;
    fs::path                                        root;
    const bool                                      bForceResetOnExecute = false;
    std::vector<fs::path>                           regularFiles;
    std::vector<std::pair<std::string, EAssetType>> importTargets;
    json                                            serializedImportConfigMap;
};
} // namespace sy::asset