#pragma once
#include <PCH.h>
#include <Asset/Asset.h>
#include <Render/Vertex.h>

namespace sy::vk
{
class VulkanContext;
class Buffer;
} // namespace sy::vk

namespace sy::render
{
class Mesh;
}

namespace sy::asset
{
class Material;
class Model : public Asset
{
public:
    struct Mesh
    {
    public:
        json Serialize() const;
        void Deserialize(const json& serialized);

    public:
        std::string Name;
        fs::path    MaterialAssetPath = {};

        Range<size_t> VerticesBlobRange;
        Range<size_t> IndicesBlobRange;
        size_t        NumVertices;
        size_t        NumIndices;
    };

public:
    Model(const fs::path& path, RefOptional<HandleManager> handleManager = std::nullopt, RefOptional<vk::VulkanContext> vulkanContext = std::nullopt);
    ~Model() override = default;

    [[nodiscard]] size_t GetTypeHash() const override { return TypeHash<Model>; }

    [[nodiscard]] std::span<const Handle<render::Mesh>> GetMeshes() const { return meshes; }
    [[nodiscard]] render::EVertexType                   GetVertexType() const { return vertexType; }
    [[nodiscard]] bool                                  IsCompressed() const { return bIsCompressed; }

    void SetVertexType(const render::EVertexType type) { vertexType = type; }
    void SetVerticesBlobSize(const size_t size) { verticesBlobSize = size; }
    void SetIndicesBlobSize(const size_t size) { indicesBlobSize = size; }
    void EnableCompression() { bIsCompressed = true; }
    void DisableCompression() { bIsCompressed = false; }

    template <typename... Args>
    void EmplaceMeshData(Args&&... args) { meshDataList.emplace_back(std::forward<Args>(args)...); }

    [[nodiscard]] json Serialize() const override;
    void               Deserialize(const json& root) override;

private:
    bool InitializeBlob(std::vector<uint8_t> blob) override;

private:
    /** Metadata */
    render::EVertexType vertexType       = render::EVertexType::PT0N;
    bool                bIsCompressed    = false;
    size_t              verticesBlobSize = 0;
    size_t              indicesBlobSize  = 0;
    std::vector<Mesh>   meshDataList     = {};

    /** Engine Instances */
    RefOptional<HandleManager>        handleManager = std::nullopt;
    RefOptional<vk::VulkanContext>    vulkanContext = std::nullopt;
    std::vector<Handle<render::Mesh>> meshes;
};
} // namespace sy::asset