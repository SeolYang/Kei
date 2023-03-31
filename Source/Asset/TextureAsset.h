#pragma once
#include <PCH.h>
#include <Asset/Asset.h>
#include <Core/Constants.h>

namespace sy::vk
{
class VulkanContext;
class Texture;
class TextureView;
class Sampler;
} // namespace sy::vk

namespace sy::asset
{
enum class ETextureCompressionMode
{
    BC4,  /** Recommend to 'Gray-scale'		#NotImplementedYet */
    BC5,  /** Recommend to 'Normal Map'		#NotImplementedYet */
    BC6H, /** Recommend to 'HDR Texture'	#NotImplementedYet */
    BC7,  /** Recommend to 'Texture'		#NotImplementedYet */
    ETC2, /** Recommend to 'Texture'		#NotImplementedYet */
    None,
};

enum class ETextureCompressionQuality
{
    Low,
    Medium,
    High
};

class Texture : public Asset
{
public:
    Texture(const fs::path& path, RefOptional<HandleManager> handleManager = std::nullopt, RefOptional<vk::VulkanContext> vulkanContext = std::nullopt);
    ~Texture() override = default;

    [[nodiscard]] size_t GetTypeHash() const override { return TypeHash<Texture>; }

    [[nodiscard]] auto GetDescriptor() const { return descriptor; }
    [[nodiscard]] auto GetCompressionMode() const { return compressionMode; }
    [[nodiscard]] auto GetCompressionQuality() const { return compressionQuality; }
    [[nodiscard]] auto GetExtent() const { return extent; }
    [[nodiscard]] auto GetFormat() const { return format; }
    [[nodiscard]] std::string_view GetSamplerAlias() const { return sampler.GetAlias(); }

    void SetCompressionMode(const ETextureCompressionMode mode) { this->compressionMode = mode; }
    void SetCompressQuality(const ETextureCompressionQuality quality) { this->compressionQuality = quality; }
    void SetExtent(const Extent2D<uint32_t> extent) { this->extent = extent; }
    void SetFormat(const VkFormat format) { this->format = format; }
    void SetSampler(const std::string_view sampler) { this->samplerAlias = sampler; }

    [[nodiscard]] json Serialize() const override;
    void               Deserialize(const nlohmann::json& serializedMetadata) override;

private:
    bool InitializeBlob(std::vector<uint8_t> blob) override;

private:
    /** Metadata */
    ETextureCompressionMode    compressionMode    = ETextureCompressionMode::None;
    ETextureCompressionQuality compressionQuality = ETextureCompressionQuality::High;
    Extent2D<uint32_t>         extent             = Extent2D<uint32_t>{1, 1};
    VkFormat                   format             = VK_FORMAT_UNDEFINED;
    std::string_view           samplerAlias       = core::constants::res::TrilinearRepeatSampler;

    /** Engine Instances */
    RefOptional<HandleManager>     handleManager = std::nullopt;
    RefOptional<vk::VulkanContext> vulkanContext = std::nullopt;
    Handle<vk::Texture>            texture       = {};
    Handle<vk::TextureView>        textureView   = {};
    Handle<vk::Sampler>            sampler       = {};
    Handle<vk::Descriptor>         descriptor    = {};
};
} // namespace sy::asset