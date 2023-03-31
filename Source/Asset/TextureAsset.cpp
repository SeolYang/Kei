#include <PCH.h>
#include <Asset/TextureAsset.h>
#include <VK/Texture.h>
#include <VK/TextureBuilder.h>
#include <VK/TextureView.h>
#include <VK/Sampler.h>
#include <VK/DescriptorManager.h>

namespace sy::asset
{
Texture::Texture(const fs::path& path, RefOptional<HandleManager> handleManager, RefOptional<vk::VulkanContext> vulkanContext) :
    Asset(path),
    handleManager(handleManager),
    vulkanContext(vulkanContext)
{
}

json Texture::Serialize() const
{
    namespace predefined_key = sy::asset::constants::metadata::key;

    auto root = Asset::Serialize();
    // #todo Unified method to serialization method.
    root[predefined_key::CompressionMode]    = magic_enum::enum_name(compressionMode);
    root[predefined_key::CompressionQuality] = magic_enum::enum_name(compressionQuality);
    root[predefined_key::Extent]             = std::make_pair(extent.width, extent.height);
    root[predefined_key::Format]             = magic_enum::enum_name(format);
    root[predefined_key::Sampler]            = this->samplerAlias;

    return root;
}

void Texture::Deserialize(const json& root)
{
    namespace predefined_key = sy::asset::constants::metadata::key;

    Asset::Deserialize(root);

    this->compressionMode = ResolveEnumFromJson(
        root,
        predefined_key::CompressionMode,
        ETextureCompressionMode::None);

    this->compressionQuality = ResolveEnumFromJson(
        root,
        predefined_key::CompressionQuality,
        ETextureCompressionQuality::Medium);

    const std::pair<decltype(extent.width), decltype(extent.height)> extentPair = root[predefined_key::Extent];

    this->extent = {extentPair.first,
                    extentPair.second};

    this->format = ResolveEnumFromJson(
        root,
        predefined_key::Format,
        VK_FORMAT_UNDEFINED);

    this->samplerAlias = root[predefined_key::Sampler];
}

bool Texture::InitializeBlob(std::vector<uint8_t> blob)
{
    if (!handleManager)
    {
        SY_ASSERT(false, "Trying to initialize texture without HandleManager.");
        return false;
    }

    if (!vulkanContext)
    {
        SY_ASSERT(false, "Trying to initialize texture without VulkanContext.");
        return false;
    }

    auto& handleManager = this->handleManager->get();
    auto& vulkanContext = this->vulkanContext->get();

    const auto name = GetName();

    this->texture = handleManager.Add<vk::Texture>(
        vk::TextureBuilder::Texture2DShaderResourceTemplate(vulkanContext)
            .SetName(name)
            .SetFormat(this->format)
            .SetExtent(extent)
            .SetDataToTransfer(VecToConstSpan(blob))
            .Build());

    if (!this->texture)
    {
        return false;
    }
    this->texture.SetAlias(name);

    this->textureView = handleManager.Add<vk::TextureView>(
        std::format("{}_View", name),
        vulkanContext,
        *(this->texture),
        VK_IMAGE_VIEW_TYPE_2D);

	this->sampler = handleManager.QueryAlias<vk::Sampler>(samplerAlias);
	if (!this->sampler)
	{
        /** #fallback #1 : Attempt to load engine default trilinear sampler. */
        this->sampler = handleManager.QueryAlias<vk::Sampler>(core::constants::res::TrilinearRepeatSampler);
	}

    auto& descriptorManager = vulkanContext.GetDescriptorManager();
    this->descriptor        = handleManager.Add<vk::Descriptor>(
        descriptorManager.RequestDescriptor(
            *(this->texture),
            *(this->textureView),
            *(this->sampler),
            vk::ETextureState::AnyShaderReadSampledImage));

    return this->texture.IsValid();
}
} // namespace sy::asset