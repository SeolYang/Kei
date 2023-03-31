#include <PCH.h>
#include <Asset/MaterialAsset.h>
#include <Asset/TextureAsset.h>

namespace sy::asset
{
Material::Material(const fs::path& path, const RefOptional<HandleManager> handleManager, const RefOptional<vk::VulkanContext> vulkanContext) :
    Asset(path),
    handleManager(handleManager),
    vulkanContext(vulkanContext)
{
    EnableIgnoreBlob();
}


json Material::Serialize() const
{
    json root                                   = Asset::Serialize();
    root[constants::metadata::key::BaseTexture] = baseTexturePath.string();
    return root;
}

void Material::Deserialize(const json& root)
{
    Asset::Deserialize(root);
    const std::string baseTexturePathStr = root[constants::metadata::key::BaseTexture];
    baseTexturePath                      = baseTexturePathStr;
}

bool Material::InitializeBlob(const std::vector<uint8_t> blob)
{
    return true;
}

void Material::EndDeserialize()
{
    if (!handleManager)
    {
        SY_ASSERT(false, "Trying to initialize model without HandleManager.");
        return;
    }

    if (!vulkanContext)
    {
        SY_ASSERT(false, "Trying to initialize model without VulkanContext.");
        return;
    }

    auto& handleManager = this->handleManager->get();
    auto& vulkanContext = this->vulkanContext->get();

    Handle<vk::Descriptor> baseTextureDescriptor = {};
    {
        const std::string baseTexturePathStr = baseTexturePath.string();
        auto              baseTextureAsset   = handleManager.QueryAlias<asset::Texture>(baseTexturePathStr);
        /** #fallback #1: Attempt to load new texture asset from file. */
        if (!baseTextureAsset)
        {
            baseTextureAsset =
                handleManager.Add<asset::Texture>(baseTexturePathStr,
                                                  handleManager,
                                                  vulkanContext);

            if (!baseTextureAsset->Initialize())
            {
                baseTextureAsset.DestroySelf();
            }
            else
            {
                baseTextureAsset.SetAlias(baseTexturePathStr);
            }
        }

        if (baseTextureAsset)
        {
            baseTextureDescriptor = baseTextureAsset->GetDescriptor();
        }
        /** #fallback #2: Attempt to load default texture descriptor from engine. */
        else
        {
            baseTextureDescriptor = handleManager.QueryAlias<vk::Descriptor>(core::constants::res::DefaultWhiteTexture);
        }
    }
}
} // namespace sy::asset