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
    const size_t sizeOfVertex = render::SizeOfVertex(config.VertexType);
    const size_t sizeOfIndex  = sizeof(render::IndexType);

    uint32_t importFlags = aiProcess_Triangulate;
    importFlags |= (config.bFlipWingdingOrder || config.bConvertToLeftHanded ? aiProcess_FlipWindingOrder : 0);
    importFlags |= (config.bFlipUVs || config.bConvertToLeftHanded ? aiProcess_FlipUVs : 0);
    importFlags |= (config.bMakeLeftHanded || config.bConvertToLeftHanded ? aiProcess_MakeLeftHanded : 0);
    importFlags |= (config.bGenUVCoords ? aiProcess_GenUVCoords : 0);
    importFlags |= (config.bGenNormals ? aiProcess_GenNormals : 0);
    importFlags |= (config.bGenSmoothNormals ? aiProcess_GenSmoothNormals : 0);
    importFlags |= (config.bCalcTagentSpace ? aiProcess_CalcTangentSpace : 0);
    importFlags |= (config.bPretransformVertices ? aiProcess_PreTransformVertices : 0);

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
        for (size_t vIdx = 0; vIdx < numMeshVertices; ++vIdx)
        {
            const size_t vertexByteOffset = +vIdx * sizeOfVertex;
            uint8_t*     vertexBase       = meshVerticesBlob.data() + vertexByteOffset;
            if (const auto colorAttributeRange = QueryRangeOfVertexAttribute(config.VertexType, render::EVertexAttributeType::Color);
                colorAttributeRange)
            {
                if (const auto colorAttribute = colorAttributeRange->CastTo<glm::vec4>(vertexBase);
                    colorAttribute)
                {
                    glm::vec4& color = colorAttribute->get();
                    color.r          = mesh->mColors[0][vIdx].r;
                    color.g          = mesh->mColors[0][vIdx].g;
                    color.b          = mesh->mColors[0][vIdx].b;
                    color.a          = mesh->mColors[0][vIdx].a;
                }
            }

            if (const auto posAttributeRange = QueryRangeOfVertexAttribute(config.VertexType, render::EVertexAttributeType::Position);
                posAttributeRange)
            {
                if (const auto posAttribute = posAttributeRange->CastTo<glm::vec3>(vertexBase);
                    posAttribute)
                {
                    glm::vec3& pos = posAttribute->get();
                    pos.x          = mesh->mVertices[vIdx].x;
                    pos.y          = mesh->mVertices[vIdx].y;
                    pos.z          = mesh->mVertices[vIdx].z;
                }
            }

            if (const auto texCoords0AttributeRange = QueryRangeOfVertexAttribute(config.VertexType, render::EVertexAttributeType::TexCoords0);
                texCoords0AttributeRange)
            {
                if (const auto texCoords0Attribute = texCoords0AttributeRange->CastTo<glm::vec2>(vertexBase);
                    texCoords0Attribute)
                {
                    glm::vec2& texCoords = texCoords0Attribute->get();
                    texCoords.s          = mesh->mTextureCoords[0][vIdx].x;
                    texCoords.t          = mesh->mTextureCoords[0][vIdx].y;
                }
            }

            if (const auto normalAttributeRange = QueryRangeOfVertexAttribute(config.VertexType, render::EVertexAttributeType::Normal);
                normalAttributeRange)
            {
                if (const auto normalAttribute = normalAttributeRange->CastTo<glm::vec3>(vertexBase))
                {
                    glm::vec3& normal = normalAttribute->get();
                    normal.x          = mesh->mNormals[vIdx].x;
                    normal.y          = mesh->mNormals[vIdx].y;
                    normal.z          = mesh->mNormals[vIdx].z;
                }
            }

            if (const auto bitangentAttributeRange = QueryRangeOfVertexAttribute(config.VertexType, render::EVertexAttributeType::Bitangent);
                bitangentAttributeRange)
            {
                if (const auto bitangentAttribute = bitangentAttributeRange->CastTo<glm::vec3>(vertexBase);
                    bitangentAttribute)
                {
                    glm::vec3& bitangent = bitangentAttribute->get();
                    bitangent.x          = mesh->mBitangents[vIdx].x;
                    bitangent.y          = mesh->mBitangents[vIdx].y;
                    bitangent.z          = mesh->mBitangents[vIdx].z;
                }
            }

            if (const auto tangentAttributeRange = QueryRangeOfVertexAttribute(config.VertexType, render::EVertexAttributeType::Tangent);
                tangentAttributeRange)
            {
                if (const auto tangentAttribute = tangentAttributeRange->CastTo<glm::vec3>(vertexBase);
                    tangentAttribute)
                {
                    glm::vec3& tangent = tangentAttribute->get();
                    tangent.x          = mesh->mTangents[vIdx].x;
                    tangent.y          = mesh->mTangents[vIdx].y;
                    tangent.z          = mesh->mTangents[vIdx].z;
                }
            }
        }

        /** Process Mesh indices */
        for (size_t fIdx = 0; fIdx < mesh->mNumFaces; ++fIdx)
        {
            const auto& face = mesh->mFaces[fIdx];
            // #assumption Mesh is always triangulated.
            SY_ASSERT(face.mNumIndices == NumIndicesPerTriangulatedFace, "Model isn't triangulated.");
            for (size_t idx = 0; idx < NumIndicesPerTriangulatedFace; ++idx)
            {
                const size_t        indexOffset = TriangulatedNumFacesToNumIndices(fIdx + idx);
                uint8_t*            indexBase   = meshIndicesBlob.data();
                const Range<size_t> indexRange{indexOffset, sizeof(render::IndexType)};

                auto& index = indexRange.CastTo<render::IndexType>(indexBase)->get();
                index       = face.mIndices[idx];
            }
        }

        /** Compress Vertices and Indices of proceed meshes.*/
        if (config.bEnableCompression)
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

        if (config.bGenMaterialPerMesh)
        {
            auto newMaterial = std::make_unique<Material>(materialPath);
            SaveJsonToFile(materialPath, newMaterial->Serialize());
        }

        newModel->EmplaceMeshData(
            meshName,
            config.bGenMaterialPerMesh ? materialPath :
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
        if (config.bEnableCompression)
        {
            newModel->EnableCompression();
        }
        newModel->SetVertexType(config.VertexType);
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
} // namespace sy::asset
