#include <PCH.h>
#include <Asset/MaterialAsset.h>
#include <Asset/TextureAsset.h>
#include <Render/Material.h>
#include <VK/Texture.h>
#include <VK/TextureView.h>
#include <VK/Sampler.h>
#include <VK/VulkanContext.h>
#include <VK/DescriptorManager.h>

namespace sy::asset
{
struct MaterialMetadata
{
    std::string BaseTexture = vk::DefaultWhiteTexture.data();
    /// ...
};

constexpr std::string_view MATERIAL_METADATA_BASE_TEXTURE = "BaseTexture";

MaterialMetadata QueryMetadata(const AssetData<render::Material>& assetData)
{
    const nlohmann::json& metadataJson = assetData.GetMetadata();
    MaterialMetadata      result;

    result.BaseTexture = metadataJson[MATERIAL_METADATA_BASE_TEXTURE];
    return result;
}

Handle<render::Material> LoadMaterialFromAsset(
    const fs::path&    path,
    HandleManager&     handleManager,
    vk::VulkanContext& vulkanContext)
{
    std::string pathStr = path.string();
    auto        handle  = handleManager.QueryAlias<render::Material>(pathStr);
    if (handle)
    {
        return handle;
    }

    const auto assetDataHandle = LoadOrCreateAssetData<render::Material>(path, handleManager);
    if (!assetDataHandle)
    {
        SY_ASSERT(false, " Failed to load material asset from {}.", pathStr);
        return {};
    }

    auto&      descriptorManager = vulkanContext.GetDescriptorManager();
    const auto metadata          = QueryMetadata(*assetDataHandle);

    const auto baseTexHandle     = LoadTexture2DFromAsset(metadata.BaseTexture, handleManager, vulkanContext);
    const auto baseTexViewHandle = handleManager.Add<vk::TextureView>(
        std::format("{}_View", metadata.BaseTexture),
        vulkanContext, *baseTexHandle,
        VK_IMAGE_VIEW_TYPE_2D);

    const auto linearSampler = handleManager.QueryAlias<vk::Sampler>(vk::LinearSamplerRepeat);

    handle = handleManager.Add<render::Material>(
        handleManager.Add<vk::Descriptor>(descriptorManager.RequestDescriptor(handleManager,
                                                                              baseTexHandle, baseTexViewHandle,
                                                                              linearSampler,
                                                                              vk::ETextureState::AnyShaderReadSampledImage)));
    handle.SetAlias(pathStr);
    return handle;
}

nlohmann::json ToMetadata(const MaterialMetadata metadata)
{
    nlohmann::json result;
    result[MATERIAL_METADATA_BASE_TEXTURE] = metadata.BaseTexture;
    return result;
}

void CreateMaterial(const fs::path& path)
{
    fs::create_directory(path.parent_path());
    const MaterialMetadata            metadata;
    const AssetData<render::Material> assetData{path, ToMetadata(metadata), {}};
    assetData.SaveMetadata();
}
} // namespace sy::asset
