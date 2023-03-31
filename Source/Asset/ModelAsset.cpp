#include <PCH.h>
#include <Asset/ModelAsset.h>
#include <Asset/MaterialAsset.h>
#include <Render/Material.h>
#include <Render/Mesh.h>
#include <Render/Vertex.h>
#include <VK/Buffer.h>
#include <VK/BufferBuilder.h>
#include <Core/Constants.h>

namespace sy::asset
{
Model::Model(const fs::path& path, const RefOptional<HandleManager> handleManager, const RefOptional<vk::VulkanContext> vulkanContext) :
    Asset(path),
    handleManager(handleManager),
    vulkanContext(vulkanContext)
{
}

bool Model::InitializeBlob(const std::vector<uint8_t> blob)
{
    if (!handleManager)
    {
        SY_ASSERT(false, "Trying to initialize model without HandleManager.");
        return false;
    }

    if (!vulkanContext)
    {
        SY_ASSERT(false, "Trying to initialize model without VulkanContext.");
        return false;
    }

    const std::string_view name = GetName();

    auto& handleManager = this->handleManager->get();
    auto& vulkanContext = this->vulkanContext->get();

    const auto verticesBlobSpan = std::span(static_cast<const uint8_t*>(blob.data()), verticesBlobSize);
    const auto indicesBlobSpan  = std::span(static_cast<const uint8_t*>(blob.data() + verticesBlobSize), indicesBlobSize);

    meshes.reserve(meshDataList.size());
    for (const auto& meshData : meshDataList)
    {
        const std::string formattedMeshName = std::format("{}_{}", GetName(), meshData.Name);

        auto mesh = handleManager.QueryAlias<render::Mesh>(formattedMeshName);
        if (!mesh)
        {
            Handle<render::Material> material;
            {
                auto materialAsset = handleManager.QueryAlias<asset::Material>(meshData.MaterialAssetPath.string());
                /** #fallback #1: Attempt to load new material asset from file. */
                if (!materialAsset)
                {
                    materialAsset =
                        handleManager.Add<asset::Material>(meshData.MaterialAssetPath,
                                                           handleManager,
                                                           vulkanContext);

                    if (!materialAsset->Initialize())
                    {
                        materialAsset.DestroySelf();
                    }
                    else
                    {
                        materialAsset.SetAlias(meshData.MaterialAssetPath.string());
                    }
                }

                if (materialAsset)
                {
                    material = materialAsset->GetMaterial();
                }
                /** #fallback #2: Attempt to load default material instance from engine. */
                else
                {
                    material = handleManager.QueryAlias<render::Material>(core::constants::res::DefaultMaterialInstance);
                }
            }

            const size_t sizeOfVertex = SizeOfVertex(GetVertexType());

            std::vector<uint8_t> meshVerticesBlob;
            std::vector<uint8_t> meshIndicesBlob;

            const auto meshVerticesBlobSubspan = verticesBlobSpan.subspan(meshData.VerticesBlobRange.Offset, meshData.VerticesBlobRange.Size);
            const auto meshIndicesBlobSubspan  = indicesBlobSpan.subspan(meshData.IndicesBlobRange.Offset, meshData.IndicesBlobRange.Size);

            if (IsCompressed())
            {
                meshVerticesBlob.resize(sizeOfVertex * meshData.NumVertices);
                meshIndicesBlob.resize(sizeof(render::IndexType) * meshData.NumIndices);

                int result = meshopt_decodeVertexBuffer(
                    meshVerticesBlob.data(),
                    meshData.NumVertices, sizeOfVertex,
                    meshVerticesBlobSubspan.data(),
                    meshVerticesBlobSubspan.size());

                if (result != 0)
                {
                    spdlog::error("Failed to decompress vertices of {}.", GetName());
                }

                result = meshopt_decodeIndexBuffer(
                    reinterpret_cast<render::IndexType*>(meshIndicesBlob.data()),
                    meshData.NumIndices,
                    meshIndicesBlobSubspan.data(),
                    meshIndicesBlobSubspan.size());

                if (result != 0)
                {
                    spdlog::error("Failed to decompress indices of {}.", GetName());
                }
            }
            else
            {
                meshVerticesBlob.assign_range(meshVerticesBlobSubspan);
                meshIndicesBlob.assign_range(meshIndicesBlobSubspan);
            }

            mesh = handleManager.Add<render::Mesh>(
                formattedMeshName,

                meshData.NumVertices,
                vk::BufferBuilder::VertexBufferTemplate(vulkanContext)
                    .SetName(std::format("{}_VertexBuffer", formattedMeshName))
                    .SetDataToTransferWithSize(VecToConstSpan(meshVerticesBlob))
                    .Build(),

                meshData.NumIndices,
                vk::BufferBuilder::IndexBufferTemplate(vulkanContext)
                    .SetName(std::format("{}_IndexBuffer", formattedMeshName))
                    .SetDataToTransferWithSize(VecToConstSpan(meshIndicesBlob))
                    .Build(),

                material);
        }

        meshes.emplace_back(mesh);
    }

    return true;
}

json Model::Serialize() const
{
    namespace predefined_key = asset::constants::metadata::key;

    json root = Asset::Serialize();

    root[predefined_key::Name]       = GetName();
    root[predefined_key::VertexType] = magic_enum::enum_name(this->vertexType);

    nlohmann::json serializedMeshDataList;
    for (const auto& meshData : meshDataList)
    {
        serializedMeshDataList.emplace_back(meshData.Serialize());
    }

    root[predefined_key::MeshDataList]    = serializedMeshDataList;
    root[predefined_key::CompressionFlag] = IsCompressed();

    root[predefined_key::VerticesBlobSize] = verticesBlobSize;
    root[predefined_key::IndicesBlobSize]  = indicesBlobSize;
    return root;
}

void Model::Deserialize(const json& root)
{
    Asset::Deserialize(root);

    namespace predefined_key = asset::constants::metadata::key;

    vertexType = ResolveEnumFromJson(root,
                                     predefined_key::VertexType,
                                     render::EVertexType::PT0N);

    const json serializeMeshDataList = root[predefined_key::MeshDataList];
    meshDataList.resize(serializeMeshDataList.size());

    size_t idx = 0;
    for (const auto& serializeMeshData : serializeMeshDataList)
    {
        meshDataList[idx].Deserialize(serializeMeshData);
        ++idx;
    }

    bIsCompressed    = root[predefined_key::CompressionFlag];
    verticesBlobSize = root[predefined_key::VerticesBlobSize];
    indicesBlobSize  = root[predefined_key::IndicesBlobSize];
}

nlohmann::json Model::Mesh::Serialize() const
{
    namespace predefined_key = asset::constants::metadata::key;

    nlohmann::json root;
    root[predefined_key::Name]          = Name;
    root[predefined_key::MaterialAsset] = this->MaterialAssetPath.string();

    json serializedVerticesRange;
    serializedVerticesRange[predefined_key::Offset] = this->VerticesBlobRange.Offset;
    serializedVerticesRange[predefined_key::Size]   = this->VerticesBlobRange.Size;
    root[predefined_key::VerticesBlobRange]         = serializedVerticesRange;

    json serializedIndicesRange;
    serializedIndicesRange[predefined_key::Offset] = this->IndicesBlobRange.Offset;
    serializedIndicesRange[predefined_key::Size]   = this->IndicesBlobRange.Size;
    root[predefined_key::IndicesBlobRange]         = serializedIndicesRange;

    root[predefined_key::NumVertices] = NumVertices;
    root[predefined_key::NumIndices]  = NumIndices;

    return root;
}

void Model::Mesh::Deserialize(const nlohmann::json& root)
{
    namespace predefined_key = asset::constants::metadata::key;

    this->Name                          = root[predefined_key::Name];
    const std::string materialAssetPath = root[predefined_key::MaterialAsset];
    this->MaterialAssetPath             = materialAssetPath;

    const json serializedVerticesRange = root[predefined_key::VerticesBlobRange];
    this->VerticesBlobRange            = Range<size_t>{
                   .Offset = serializedVerticesRange[predefined_key::Offset],
                   .Size   = serializedVerticesRange[predefined_key::Size]};

    const json serializedIndicesRange = root[predefined_key::IndicesBlobRange];
    this->IndicesBlobRange            = Range<size_t>{
                   .Offset = serializedIndicesRange[predefined_key::Offset],
                   .Size   = serializedIndicesRange[predefined_key::Size]};

    NumVertices = root[predefined_key::NumVertices];
    NumIndices  = root[predefined_key::NumIndices];
}
} // namespace sy::asset
