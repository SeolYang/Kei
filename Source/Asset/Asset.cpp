#include <PCH.h>
#include <Asset/Asset.h>

namespace sy::asset
{
Asset::Asset(const fs::path& path) :
    NamedType(ConvertToExtensionless(path).string()),
    originPath(path),
    assetPath(ConvertToAssetPath(path)),
    blobPath(ConvertToBlobPath(path)),
    extensionlessPath(GetName())
{
}

bool Asset::Initialize()
{
    if (bInitialized)
    {
        SY_ASSERT(false, "Trying to intialize, already initialized asset!");
        return false;
    }

    BeginInit();
    {
        if (!bIsExternalFormat)
        {
            if (!fs::exists(assetPath))
            {
                spdlog::error("Asset {} doest not exist.", assetPath.string());
                return false;
            }

            BeginDeserialize();
            {
                const json root = LoadJsonFromFile(assetPath);
                Deserialize(root);
            }
            EndDeserialize();

            if (!bIgnoreBlob)
            {
                if (!fs::exists(blobPath))
                {
                    spdlog::error("Blob {} does not exist.", blobPath.string());
                    return false;
                }

                BeginInitBlob();
                {
                    const auto blob = LoadBlobFromFile(blobPath);
                    if (!InitializeBlob(blob))
                    {
                        spdlog::error("Failed to initialize blob {}.", blobPath.string());
                        return false;
                    }
                }
                EndInitBlob();
            }
        }
        else
        {
            if (!fs::exists(originPath))
            {
                spdlog::error("External asset {} does not exist.", originPath.string());
                return false;
            }

            BeginInitExternal();
            {
                if (!InitializeExternal())
                {
                    spdlog::error("Failed to initialize external asset {}.", assetPath.string());
                    return false;
                }
            }
            EndInitExternal();
        }
    }
    EndInit();

    MarkAsInitialized();
    return true;
}

json Asset::Serialize() const
{
    json root;
    root[constants::metadata::key::TypeHash] = GetTypeHash();
    return root;
}

void Asset::Deserialize(const json& root)
{
    /** Asset type validation check! */
    const auto typeHashItr = root.find(constants::metadata::key::TypeHash);
    if (typeHashItr == root.end())
    {
        SY_ASSERT(false, "Type hash does not found from metadata.");
        return;
    }

    if (*typeHashItr != GetTypeHash())
    {
        SY_ASSERT(false, "Type hash does not match! Found: {}, Expected: {}", *typeHashItr, GetTypeHash());
        return;
    }
}

} // namespace sy::asset