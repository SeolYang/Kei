#include <PCH.h>
#include <Asset/TextureAsset.h>
#include <VK/Texture.h>
#include <VK/TextureBuilder.h>
#include <VK/TextureView.h>
#include <VK/Sampler.h>
#include <VK/DescriptorManager.h>
#include <ktx.h>
#include <ktxvulkan.h>

namespace sy::asset
{
Texture::Texture(const fs::path& path, RefOptional<HandleManager> handleManager, RefOptional<vk::VulkanContext> vulkanContext) :
    Asset(path),
    handleManager(handleManager),
    vulkanContext(vulkanContext)
{
    MarkAsExternalFormat();
    AllowUsingMetadataForExternalFormat();
}

json Texture::Serialize() const
{
    namespace predefined_key = sy::asset::constants::metadata::key;

    auto root = Asset::Serialize();
    // #todo Unified method to serialization method.
    root[predefined_key::CompressionMode]    = magic_enum::enum_name(compressionMode);
    root[predefined_key::CompressionQuality] = magic_enum::enum_name(compressionQuality);
    root[predefined_key::Quality]            = magic_enum::enum_name(quality);
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
        ETextureCompressionQuality::High);

    this->quality = ResolveEnumFromJson(
        root,
        predefined_key::Quality,
        ETextureQuality::High);

    const std::pair<decltype(extent.width), decltype(extent.height)> extentPair = root[predefined_key::Extent];

    this->extent = {extentPair.first,
                    extentPair.second};

    this->format = ResolveEnumFromJson(
        root,
        predefined_key::Format,
        VK_FORMAT_UNDEFINED);

    this->samplerAlias = ResolveValueFromJson(
        root, predefined_key::Sampler,
        core::constants::res::TrilinearRepeatSampler);
}

bool Texture::InitializeExternal()
{
    using UniqueKtxTexture2 = std::unique_ptr<ktxTexture2, std::function<void(ktxTexture2*)>>;
    UniqueKtxTexture2 externalTexture;
    {
        const std::string pathStr = GetOriginPath().string();

        ktxTexture2*           raw    = nullptr;
        const ktx_error_code_e result = ktxTexture_CreateFromNamedFile(
            pathStr.c_str(),
            KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT,
            reinterpret_cast<ktxTexture**>(&raw));
        if (result != KTX_SUCCESS)
        {
            spdlog::error("Failed to load ktx texture from {}. Error: {}", pathStr, magic_enum::enum_name<ktx_error_code_e>(result));
            return false;
        }

        externalTexture = UniqueKtxTexture2(raw, [](ktxTexture2* ptr) {
            ktxTexture_Destroy(ktxTexture(ptr));
        });
    }

    ktx_transcode_fmt_e targetFormat = KTX_TTF_RGBA32;
    switch (compressionMode)
    {
        case ETextureCompressionMode::BC1:
            targetFormat = KTX_TTF_BC1_RGB;
            break;
        case ETextureCompressionMode::BC3:
            targetFormat = KTX_TTF_BC3_RGBA;
            break;
        case ETextureCompressionMode::BC7:
            targetFormat = KTX_TTF_BC7_RGBA;
            break;
        case ETextureCompressionMode::BC4:
            targetFormat = KTX_TTF_BC4_R;
            break;
        case ETextureCompressionMode::BC5:
            targetFormat = KTX_TTF_BC5_RG;
            break;
    }

    // #todo check support format

    if (ktxTexture2_NeedsTranscoding(externalTexture.get()))
    {
        ktxTexture2_TranscodeBasis(externalTexture.get(), targetFormat, 0);
    }

    SetFormat(static_cast<VkFormat>(externalTexture->vkFormat));

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

    auto textureBuilder = vk::TextureBuilder::Texture2DShaderResourceTemplate(vulkanContext)
                              .SetName(name)
                              .SetFormat(this->format)
                              .SetExtent(extent)
                              .SetDataToTransfer(std::span{
                                  reinterpret_cast<const uint8_t*>(externalTexture->pData),
                                  externalTexture->dataSize})
                              .SetTargetInitialState(vk::ETextureState::AnyShaderReadSampledImage)
                              .SetMips(externalTexture->numLevels)
                              .SetArrayLayers(externalTexture->numLayers);

    const uint32_t mipLevels = externalTexture->numLevels;
    if (mipLevels > 1)
    {
        const auto baseExtent = extent;
        for (uint32_t mip = 0; mip < externalTexture->numLevels; ++mip)
        {
            const auto mipExtent       = CalculateMipExtent(baseExtent, mip);
            size_t     mipBufferOffset = 0;
            ktxTexture_GetImageOffset(ktxTexture(externalTexture.get()), mip, 0, 0, &mipBufferOffset);
            const VkBufferImageCopy copyInfo{
                .bufferOffset     = mipBufferOffset,
                .imageSubresource = {
                    .aspectMask     = vk::FormatToImageAspect(format),
                    .mipLevel       = mip,
                    .baseArrayLayer = 0,
                    .layerCount     = 1},
                .imageExtent = {mipExtent.width, mipExtent.height, 1}};

			textureBuilder.AddCopyInfo(copyInfo);
        }
    }

    // #todo into account mips, see "KTX-Software/vkloader.c/ktxTexture_VkUploadEx"
    this->texture = handleManager.Add<vk::Texture>(textureBuilder.Build());
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