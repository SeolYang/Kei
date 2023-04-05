#include <PCH.h>
#include <Asset/AssetImportConfig.h>
#include <Asset/Constants.h>

namespace sy::asset
{
json AssetImportConfig::Serialize() const
{
    json root;
    root[constants::metadata::key::ReadyToImport] = bReadyToImport;
    return root;
}

void AssetImportConfig::Deserialize(const json& root)
{
    bReadyToImport = ResolveValueFromJson(root, constants::metadata::key::ReadyToImport, true);
}
}