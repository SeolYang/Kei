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
    ExtractRegularFilesInRootAssetDirectory();
    ExtractImportTargetsFromRegularFiles();
    UpdateUnseenImportTargetsToConfigMap();
    FilteringImportTargets();
    ImportTargetAssets();
}

void AssetImporter::ExtractRegularFilesInRootAssetDirectory()
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
            importTargets.emplace_back(std::make_pair(regularFilePathStr, *assetType));
        }
    }
}

void AssetImporter::UpdateUnseenImportTargetsToConfigMap()
{
    for (const auto& importTarget : importTargets)
    {
        const auto& importTargetPath      = importTarget.first;
        const auto  importTargetAssetType = importTarget.second;
        if (!serializedImportConfigMap.contains(importTargetPath))
        {
            serializedImportConfigMap[importTargetPath] = GetSerializedDefaultConfig(importTargetAssetType);
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

    serializedConfig[constants::metadata::key::RequireReimportAsset] = true;
    return serializedConfig;
}

void AssetImporter::FilteringImportTargets()
{
    importTargets.erase(std::remove_if(importTargets.begin(), importTargets.end(),
                                       [this](const auto& importTarget) {
                                           const json& config                  = serializedImportConfigMap[importTarget.first];
                                           const bool  bIsNotNecessaryToImport = !ResolveValueFromJson(config, constants::metadata::key::RequireReimportAsset, true);
                                           return bIsNotNecessaryToImport;
                                       }),
                        importTargets.end());
}

void AssetImporter::ImportTargetAssets()
{
    spdlog::info("#AssetImportTargetFiles: {}", importTargets.size());
    // #todo Impl as Multi-threaded
    for (const auto& importTarget : importTargets)
    {
        const auto& path      = importTarget.first;
        const auto  assetType = importTarget.second;
        auto&       config    = serializedImportConfigMap[path];
        ImportAsset(path, assetType, config);
    }
}

void AssetImporter::ImportAsset(const std::string_view path, const EAssetType assetType, json& config)
{
    auto begin = chrono::high_resolution_clock::now();
    switch (assetType)
    {
        case EAssetType::Texture:
            ImportTextureAsset(path, config);
            break;

        case EAssetType::Model:
            ImportModelAsset(path, config);
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
    config[constants::metadata::key::RequireReimportAsset] = false;
}

void AssetImporter::ExportAssetImportConfigs()
{
    SaveJsonToFile(constants::path::AssetImportConfigs, serializedImportConfigMap);
}

} // namespace sy::asset