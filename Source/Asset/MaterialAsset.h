#pragma once
#include <PCH.h>
#include <Asset/Asset.h>
#include <Core/Constants.h>

namespace sy::render
{
class Material;
class Texture;
} // namespace sy::render

namespace sy::asset
{
class Texture;
class Material : public Asset
{
public:
    Material(const fs::path& path, RefOptional<HandleManager> handleManager= std::nullopt, RefOptional<vk::VulkanContext> vulkanContext = std::nullopt);
    ~Material() override = default;

    [[nodiscard]] size_t GetTypeHash() const override { return TypeHash<Material>; }

    [[nodiscard]] Handle<render::Material> GetMaterial() const { return material; }

    [[nodiscard]] json Serialize() const override;
    void               Deserialize(const json& root) override;

protected:
    void EndDeserialize() override;

private:
    bool InitializeBlob(std::vector<uint8_t> blob) override;

private:
    fs::path baseTexturePath = core::constants::res::DefaultWhiteTexture;

    /** Engine Instances */
    RefOptional<HandleManager>     handleManager = std::nullopt;
    RefOptional<vk::VulkanContext> vulkanContext = std::nullopt;
    Handle<render::Material>       material      = {};
};
} // namespace sy::asset