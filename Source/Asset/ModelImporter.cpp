#include <PCH.h>
#include <Asset/ModelImporter.h>
#include <Asset/ModelAsset.h>
#include <Asset/MaterialAsset.h>
#include <Core/Constants.h>

namespace sy::asset
{
constexpr size_t NumIndicesPerTriangulatedFace = 3;
inline size_t    TriangulatedNumFacesToNumIndices(const size_t numFaces)
{
    return numFaces * NumIndicesPerTriangulatedFace;
}

/**
* #todo Impl for scene hierarchy(ignore scene hierarchy if config.bPretransformVertices enabled)
*/
bool ModelImporter::Import(const fs::path& path, const ModelImportConfig config)
{
    bool         bSucceed     = true;
    const size_t sizeOfVertex = render::SizeOfVertex(config.GetVertexType());
    const size_t sizeOfIndex  = sizeof(render::IndexType);

    uint32_t importFlags = aiProcess_Triangulate;
    importFlags |= (config.IsFlipWindingOrder() ? aiProcess_FlipWindingOrder : 0);
    importFlags |= (config.IsFlipTextureCoordinates() ? aiProcess_FlipUVs : 0);
    importFlags |= (config.IsMakeLeftHanded() ? aiProcess_MakeLeftHanded : 0);
    importFlags |= (config.IsConvertToLeftHanded() ? aiProcess_ConvertToLeftHanded : 0);
    importFlags |= (config.IsGenerateTextureCoordinates() ? aiProcess_GenUVCoords : 0);
    importFlags |= (config.IsGenerateNormals() ? aiProcess_GenNormals : 0);
    importFlags |= (config.IsGenerateSmoothNormals() ? aiProcess_GenSmoothNormals : 0);
    importFlags |= (config.IsCalculateTangentSpace() ? aiProcess_CalcTangentSpace : 0);
    importFlags |= (config.IsPretransformVertices() ? aiProcess_PreTransformVertices : 0);

    Assimp::Importer importer;
    std::string      pathStr = path.string();
    const aiScene*   scene   = importer.ReadFile(pathStr, importFlags);
    if (scene == nullptr)
    {
        spdlog::error("Failed load scene using assimp from {}.", pathStr);
        return false;
    }

    std::unique_ptr<Model> newModel = std::make_unique<Model>(path);

    const auto meshes = std::span{scene->mMeshes, scene->mNumMeshes};

    /** Vertices/Indices Buffer Reservation. */
    std::vector<uint8_t> modelVerticesBlob;
    size_t               requiredVertexBufferSize = 0;
    std::vector<uint8_t> modelIndicesBlob;
    size_t               requiredIndexBufferSize = 0;
    for (const aiMesh* mesh : meshes)
    {
        requiredVertexBufferSize += (sizeOfVertex * mesh->mNumVertices);
        requiredIndexBufferSize += sizeof(render::IndexType) * mesh->mNumFaces * 3;
    }
    modelVerticesBlob.reserve(requiredVertexBufferSize);
    modelIndicesBlob.reserve(requiredIndexBufferSize);

    /** Name map to prevent duplicate mesh names */
    robin_hood::unordered_map<std::string, size_t> meshNameMap;

    // Process Model blob
    for (const aiMesh* mesh : meshes)
    {
        const size_t numMeshVertices = mesh->mNumVertices;
        const size_t numMeshIndices  = TriangulatedNumFacesToNumIndices(mesh->mNumFaces);

        std::vector<uint8_t> meshVerticesBlob;
        meshVerticesBlob.resize(sizeOfVertex * numMeshVertices);
        std::vector<uint8_t> meshIndicesBlob;
        meshIndicesBlob.resize(sizeof(render::IndexType) * numMeshIndices);

        /** Process Mesh vertices */
        uint8_t*   verticesBase          = meshVerticesBlob.data();
        const auto targetModelVertexType = config.GetVertexType();
        for (size_t vIdx = 0; vIdx < numMeshVertices; ++vIdx)
        {
            if (const auto colorAttributeRange = QueryRangeOfVertexAttribute(targetModelVertexType, render::EVertexAttributeType::Color);
                colorAttributeRange)
            {
                if (const auto colorAttribute = colorAttributeRange->CastTo<glm::vec4>(verticesBase);
                    colorAttribute)
                {
                    glm::vec4& color = colorAttribute->get();
                    color.r          = mesh->mColors[0][vIdx].r;
                    color.g          = mesh->mColors[0][vIdx].g;
                    color.b          = mesh->mColors[0][vIdx].b;
                    color.a          = mesh->mColors[0][vIdx].a;
                }
            }

            if (const auto posAttributeRange = QueryRangeOfVertexAttribute(targetModelVertexType, render::EVertexAttributeType::Position);
                posAttributeRange)
            {
                if (const auto posAttribute = posAttributeRange->CastTo<glm::vec3>(verticesBase);
                    posAttribute)
                {
                    glm::vec3& pos = posAttribute->get();
                    pos.x          = mesh->mVertices[vIdx].x;
                    pos.y          = mesh->mVertices[vIdx].y;
                    pos.z          = mesh->mVertices[vIdx].z;
                }
            }

            if (const auto texCoords0AttributeRange = QueryRangeOfVertexAttribute(targetModelVertexType, render::EVertexAttributeType::TexCoords0);
                texCoords0AttributeRange)
            {
                if (const auto texCoords0Attribute = texCoords0AttributeRange->CastTo<glm::vec2>(verticesBase);
                    texCoords0Attribute)
                {
                    glm::vec2& texCoords = texCoords0Attribute->get();
                    texCoords.s          = mesh->mTextureCoords[0][vIdx].x;
                    texCoords.t          = mesh->mTextureCoords[0][vIdx].y;
                }
            }

            if (const auto normalAttributeRange = QueryRangeOfVertexAttribute(targetModelVertexType, render::EVertexAttributeType::Normal);
                normalAttributeRange)
            {
                if (const auto normalAttribute = normalAttributeRange->CastTo<glm::vec3>(verticesBase))
                {
                    glm::vec3& normal = normalAttribute->get();
                    normal.x          = mesh->mNormals[vIdx].x;
                    normal.y          = mesh->mNormals[vIdx].y;
                    normal.z          = mesh->mNormals[vIdx].z;
                }
            }

            if (const auto bitangentAttributeRange = QueryRangeOfVertexAttribute(targetModelVertexType, render::EVertexAttributeType::Bitangent);
                bitangentAttributeRange)
            {
                if (const auto bitangentAttribute = bitangentAttributeRange->CastTo<glm::vec3>(verticesBase);
                    bitangentAttribute)
                {
                    glm::vec3& bitangent = bitangentAttribute->get();
                    bitangent.x          = mesh->mBitangents[vIdx].x;
                    bitangent.y          = mesh->mBitangents[vIdx].y;
                    bitangent.z          = mesh->mBitangents[vIdx].z;
                }
            }

            if (const auto tangentAttributeRange = QueryRangeOfVertexAttribute(targetModelVertexType, render::EVertexAttributeType::Tangent);
                tangentAttributeRange)
            {
                if (const auto tangentAttribute = tangentAttributeRange->CastTo<glm::vec3>(verticesBase);
                    tangentAttribute)
                {
                    glm::vec3& tangent = tangentAttribute->get();
                    tangent.x          = mesh->mTangents[vIdx].x;
                    tangent.y          = mesh->mTangents[vIdx].y;
                    tangent.z          = mesh->mTangents[vIdx].z;
                }
            }

            verticesBase += sizeOfVertex;
        }

        /** Process Mesh indices */
        uint8_t* indicesBase = meshIndicesBlob.data();
        for (size_t fIdx = 0; fIdx < mesh->mNumFaces; ++fIdx)
        {
            const auto& face = mesh->mFaces[fIdx];
            // #assumption Mesh is always triangulated.
            SY_ASSERT(face.mNumIndices == NumIndicesPerTriangulatedFace, "Model isn't triangulated.");
            for (size_t idx = 0; idx < NumIndicesPerTriangulatedFace; ++idx)
            {
                const Range<size_t> indexRange{0, sizeof(render::IndexType)};

                auto& index = indexRange.CastTo<render::IndexType>(indicesBase)->get();
                index       = face.mIndices[idx];

                indicesBase += sizeof(render::IndexType);
            }
        }

        /** Compress Vertices and Indices of proceed meshes.*/
        if (config.IsCompressionEnabled())
        {
            /** Vertex Compression */
            {
                std::vector<uint8_t> compressedVertices(meshopt_encodeVertexBufferBound(numMeshVertices, sizeOfVertex));
                compressedVertices.resize(meshopt_encodeVertexBuffer(
                    compressedVertices.data(),
                    compressedVertices.size(),
                    meshVerticesBlob.data(),
                    numMeshVertices, sizeOfVertex));

                meshVerticesBlob = std::move(compressedVertices);
            }

            /** Index Compression */
            {
                std::vector<uint8_t> compressedIndices(meshopt_encodeIndexBufferBound(numMeshIndices, numMeshVertices));
                compressedIndices.resize(meshopt_encodeIndexBuffer(
                    compressedIndices.data(), compressedIndices.size(),
                    reinterpret_cast<render::IndexType*>(meshIndicesBlob.data()),
                    numMeshIndices));

                meshIndicesBlob = std::move(compressedIndices);
            }
        }

        /** Setup mesh metadata */
        const Range<size_t> verticesBlobRange = {modelVerticesBlob.size(),
                                                 meshVerticesBlob.size()};

        const Range<size_t> indicesBlobRange = {modelIndicesBlob.size(),
                                                meshIndicesBlob.size()};

        std::string meshName = mesh->mName.C_Str();
        if (!meshNameMap.contains(meshName))
        {
            meshNameMap[meshName] = 0;
        }
        else
        {
            meshName = std::format("{}_{}", meshName, meshNameMap[meshName]++);
        }

        const std::string materialPath = std::format("{}_{}.material", newModel->GetExtensionlessPath().string(), meshName);

        if (config.IsGenerateMaterialPerMesh())
        {
            auto newMaterial = std::make_unique<Material>(materialPath);
            SaveJsonToFile(materialPath, newMaterial->Serialize());
        }

        newModel->EmplaceMeshData(
            meshName,
            config.IsGenerateMaterialPerMesh() ? materialPath :
                                                 core::constants::res::DefaultMaterialInstance,
            verticesBlobRange,
            indicesBlobRange,
            numMeshVertices,
            numMeshIndices);

        meshVerticesBlob.shrink_to_fit();
        meshIndicesBlob.shrink_to_fit();

        modelVerticesBlob.append_range(meshVerticesBlob);
        modelIndicesBlob.append_range(meshIndicesBlob);
    }

    /** Cleanup */
    modelVerticesBlob.shrink_to_fit();
    modelIndicesBlob.shrink_to_fit();

    if (bSucceed)
    {
        /** Setup Asset Metadata */
        if (config.IsCompressionEnabled())
        {
            newModel->EnableCompression();
        }
        newModel->SetVertexType(config.GetVertexType());
        newModel->SetVerticesBlobSize(modelVerticesBlob.size());
        newModel->SetIndicesBlobSize(modelIndicesBlob.size());
        SaveJsonToFile(newModel->GetPath(), newModel->Serialize());

        std::vector<uint8_t> unifiedBlob;
        unifiedBlob.reserve(modelVerticesBlob.size() + modelIndicesBlob.size());
        unifiedBlob.append_range(modelVerticesBlob);
        unifiedBlob.append_range(modelIndicesBlob);
        SaveBlobToFile(newModel->GetBlobPath(), unifiedBlob);
    }

    importer.FreeScene();
    return bSucceed;
}

json ModelImportConfig::Serialize() const
{
    namespace key = constants::metadata::key;

    json root;
    root[key::VertexType]                 = magic_enum::enum_name(vertexType);
    root[key::GenerateMaterialPerMesh]    = bGenMaterialPerMesh;
    root[key::Compression]                = bEnableCompression;
    root[key::ConvertToLeftHanded]        = bConvertToLeftHanded;
    root[key::FlipWindingOrder]           = bFlipWingdingOrder;
    root[key::FlipTextureCoordinates]     = bFlipUVs;
    root[key::MakedLeftHanded]            = bMakeLeftHanded;
    root[key::GenerateTextureCoordinates] = bGenUVCoords;
    root[key::GenerateNormals]            = bGenNormals;
    root[key::GenerateSmoothNormals]      = bGenSmoothNormals;
    root[key::CalculateTangentSpace]      = bCalcTagentSpace;
    root[key::PretransformVertices]       = bPretransformVertices;
    return root;
}

void ModelImportConfig::Deserialize(const json& root)
{
    namespace key = constants::metadata::key;

    vertexType            = ResolveEnumFromJson(root, key::VertexType, render::EVertexType::PT0N);
    bGenMaterialPerMesh   = ResolveValueFromJson(root, key::GenerateMaterialPerMesh, false);
    bEnableCompression    = ResolveValueFromJson(root, key::Compression, false);
    bConvertToLeftHanded  = ResolveValueFromJson(root, key::ConvertToLeftHanded, false);
    bFlipWingdingOrder    = ResolveValueFromJson(root, key::FlipWindingOrder, false);
    bFlipUVs              = ResolveValueFromJson(root, key::FlipTextureCoordinates, false);
    bMakeLeftHanded       = ResolveValueFromJson(root, key::MakedLeftHanded, false);
    bGenUVCoords          = ResolveValueFromJson(root, key::GenerateTextureCoordinates, false);
    bGenNormals           = ResolveValueFromJson(root, key::GenerateNormals, false);
    bGenSmoothNormals     = ResolveValueFromJson(root, key::GenerateSmoothNormals, false);
    bCalcTagentSpace      = ResolveValueFromJson(root, key::CalculateTangentSpace, false);
    bPretransformVertices = ResolveValueFromJson(root, key::PretransformVertices, false);
}

} // namespace sy::asset
