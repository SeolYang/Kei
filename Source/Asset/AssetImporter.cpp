#include <PCH.h>
#include <Asset/AssetImporter.h>

namespace sy::asset
{
AssetImporter::AssetImporter(vk::VulkanContext& vulkanContext, const bool bForceResetOnExecute) :
    vulkanContext(vulkanContext),
    bForceResetOnExecute(bForceResetOnExecute)
{
}

void AssetImporter::Execute()
{
    LoadAssetImportConifgsFromFile();
    ForceResetOnExecute();
    ImportAssetsFromRootAssetDirectory();
    ExportAssetImportConfigs();
}

void AssetImporter::LoadAssetImportConifgsFromFile()
{
    serializedImportConfigMap = LoadJsonFromFile(constants::path::AssetImportConfigs);
}

void AssetImporter::ForceResetOnExecute()
{
    if (bForceResetOnExecute)
    {
        serializedImportConfigMap.clear();
    }
}

void AssetImporter::ImportAssetsFromRootAssetDirectory()
{
    spdlog::info("Import Assets from asset root.");
    ExtractRegularFilesFromRootAssetDirectory();
    ExtractImportTargetsFromRegularFiles();
    UpdateUnseenImportTargetsToConfigMap();
    FilteringReadyToImportTargets();
    ImportTargetAssets();
}

void AssetImporter::ExtractRegularFilesFromRootAssetDirectory()
{
    const fs::recursive_directory_iterator directoryItr{constants::path::AssetRootRelative};
    for (const auto& entry : directoryItr)
    {
        if (entry.is_regular_file())
        {
            regularFiles.emplace_back(entry.path());
        }
    }
}

void AssetImporter::ExtractImportTargetsFromRegularFiles()
{
    importTargets.reserve(regularFiles.size());
    for (const fs::path& regularFilePath : regularFiles)
    {
        const std::string regularFilePathStr = regularFilePath.string();
        const auto        assetType          = FileExtensionToAssetType(NormalizeExtension(regularFilePath.extension().string()));
        if (assetType)
        {
            importTargets.emplace_back(regularFilePathStr, *assetType);
        }
    }
}

void AssetImporter::UpdateUnseenImportTargetsToConfigMap()
{
    for (const auto& importTarget : importTargets)
    {
        if (!serializedImportConfigMap.contains(importTarget.Path))
        {
            serializedImportConfigMap[importTarget.Path] = GetSerializedDefaultConfig(importTarget.AssetType);
        }
    }
}

json AssetImporter::GetSerializedDefaultConfig(const EAssetType assetType)
{
    json serializedConfig;
    switch (assetType)
    {
        case EAssetType::Texture:
            serializedConfig = TextureImportConfig().Serialize();
            break;
        case EAssetType::Model:
        default:
            serializedConfig = ModelImportConfig().Serialize();
            break;
    }

    return serializedConfig;
}

void AssetImporter::FilteringReadyToImportTargets()
{
    importTargets.erase(std::remove_if(importTargets.begin(), importTargets.end(),
                                       [this](const auto& importTarget) {
                                           const json& config                  = serializedImportConfigMap[importTarget.Path];
                                           const bool  bIsReadyToImport = !ResolveValueFromJson(config, constants::metadata::key::ReadyToImport, true);
                                           return bIsReadyToImport;
                                       }),
                        importTargets.end());
}

void AssetImporter::ImportTargetAssets()
{
    spdlog::info("#AssetImportTargetFiles: {}", importTargets.size());
    // #todo Impl as Multi-threaded
    for (const auto& importTarget : importTargets)
    {
        ImportAsset(importTarget, serializedImportConfigMap[importTarget.Path]);
    }
}

void AssetImporter::ImportAsset(const ImportTarget& importTarget, json& config)
{
    auto begin = chrono::high_resolution_clock::now();
    switch (importTarget.AssetType)
    {
        case EAssetType::Texture:
            ImportTextureAsset(importTarget.Path, config);
            break;

        case EAssetType::Model:
            ImportModelAsset(importTarget.Path, config);
            break;
    }

    FinalizeAssetImport(config);
    auto elapsed = chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now() - begin);
    spdlog::info(" tooks(ms): {}", elapsed.count());
}

void AssetImporter::ImportTextureAsset(const fs::path& path, const json& serializedConfig)
{
    spdlog::info("Import Texture Asset \"{}\"...", path.string());
    if (!TextureImporter::Import2D(vulkanContext,
                                   path,
                                   DeserializeTextureImportConfig(serializedConfig)))
    {
        spdlog::error("Failed to import Texture Asset \"{}\".", path.string());
    }
}

void AssetImporter::ImportModelAsset(const fs::path& path, const json& serializedConfig)
{
    spdlog::info("Import Model Asset \"{}\"...", path.string());
    if (!ModelImporter::Import(path,
                               DeserializeModelImportConfig(serializedConfig)))
    {
        spdlog::error("Failed to import Model Asset \"{}\".", path.string());
    }
}

TextureImportConfig AssetImporter::DeserializeTextureImportConfig(const json& serializedConfig)
{
    TextureImportConfig config;
    config.Deserialize(serializedConfig);
    return config;
}

ModelImportConfig AssetImporter::DeserializeModelImportConfig(const json& serializedConfig)
{
    ModelImportConfig config;
    config.Deserialize(serializedConfig);
    return config;
}

void AssetImporter::FinalizeAssetImport(json& config)
{
    config[constants::metadata::key::ReadyToImport] = false;
}

void AssetImporter::ExportAssetImportConfigs()
{
    SaveJsonToFile(constants::path::AssetImportConfigs, serializedImportConfigMap);
}

} // namespace sy::asset