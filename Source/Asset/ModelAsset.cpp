#include <PCH.h>
#include <Asset/ModelAsset.h>
#include <VK/Buffer.h>

namespace sy::asset
{
	constexpr std::string_view MODEL_METADATA_COMPRESSION_MODE = "Compression";
	constexpr std::string_view MODEL_METADATA_MESH_METADATA_LIST = "MeshMetas";
	constexpr std::string_view MODEL_METADATA_MESH_VERTEX_DATA_SIZE = "VertexDataSize";
	constexpr std::string_view MODEL_METADATA_MESH_INDEX_DATA_SIZE = "IndexDataSize";
	constexpr std::string_view MESH_METADATA_NAME = "Name";
	constexpr std::string_view MESH_METADATA_VERTEX_OFFSET = "VertexOffset";
	constexpr std::string_view MESH_METADATA_VERTEX_SIZE = "VertexSize";
	constexpr std::string_view MESH_METADATA_INDEX_OFFSET = "IndexOffset";
	constexpr std::string_view MESH_METADATA_INDEX_SIZE = "IndexSize";
	constexpr uint32_t MODEL_ASSET_VERSION = 0;

	std::vector<nlohmann::json> SerializeMeshMeatadataList(const std::span<const MeshMetadata> metadataList)
	{
		std::vector<nlohmann::json> result;
		result.reserve(metadataList.size());
		for (const auto& metadata : metadataList)
		{
			nlohmann::json metadataJson;
			metadataJson[MESH_METADATA_NAME] = metadata.Name;
			metadataJson[MESH_METADATA_VERTEX_OFFSET] = metadata.VertexOffset;
			metadataJson[MESH_METADATA_VERTEX_SIZE] = metadata.VertexSize;
			metadataJson[MESH_METADATA_INDEX_OFFSET] = metadata.IndexOffset;
			metadataJson[MESH_METADATA_INDEX_SIZE] = metadata.IndexSize;
			result.emplace_back(metadataJson);
		}

		return result;
	}

	std::vector<MeshMetadata> DeSerializeMeshMetadataList(nlohmann::json metadataList)
	{
		SY_ASSERT(metadataList.is_array(), "Invalid json object to deserialize mesh metadata list.");

		std::vector<MeshMetadata> result;
		result.reserve(metadataList.size());
		for (const auto& metadata : metadataList)
		{
			result.emplace_back(
				metadata[MESH_METADATA_NAME],
				metadata[MESH_METADATA_VERTEX_OFFSET], metadata[MESH_METADATA_VERTEX_SIZE],
				metadata[MESH_METADATA_INDEX_OFFSET], metadata[MESH_METADATA_INDEX_SIZE]);
		}

		return result;
	}

	std::optional<ModelMetadata> ParseModelMetadata(const Asset& asset)
	{
		ModelMetadata metadata;
		const auto metadataJson = nlohmann::json::parse(asset.Metadata);

		const std::string compressionStr = metadataJson[MODEL_METADATA_COMPRESSION_MODE];
		const auto compression = magic_enum::enum_cast<ECompressionMode>(compressionStr);
		if (!compression)
		{
			SY_ASSERT(false, "Invalid model compression mode from asset metadata.");
			return std::nullopt;
		}
		metadata.CompressionMode = compression.value();
		metadata.MeshMetadataList = DeSerializeMeshMetadataList(metadataJson[MODEL_METADATA_MESH_METADATA_LIST]);
		metadata.VertexDataSize = metadataJson[MODEL_METADATA_MESH_VERTEX_DATA_SIZE];
		metadata.IndexDataSize = metadataJson[MODEL_METADATA_MESH_INDEX_DATA_SIZE];

		return metadata;
	}

	std::optional<Asset> Pack(const ModelMetadata& metadata, const void* vertices, const uint32_t* indices)
	{
		/** @todo mesh optimization options */
		if (vertices == nullptr)
		{
			SY_ASSERT(false, "Invalid vertices data.");
			return std::nullopt;
		}

		if (indices == nullptr)
		{
			SY_ASSERT(false, "Invalid indices data.");
		}

		nlohmann::json metadataJson;

		if (!CheckModelCompressionSupport(metadata.CompressionMode))
		{
			SY_ASSERT(false, "Unsupported compression mode for model asset.");
			return std::nullopt;
		}
		metadataJson[MODEL_METADATA_COMPRESSION_MODE] = magic_enum::enum_name<ECompressionMode>(metadata.CompressionMode);
		metadataJson[MODEL_METADATA_MESH_METADATA_LIST] = SerializeMeshMeatadataList(metadata.MeshMetadataList);

		metadataJson[MODEL_METADATA_MESH_VERTEX_DATA_SIZE] = metadata.VertexDataSize;
		metadataJson[MODEL_METADATA_MESH_INDEX_DATA_SIZE] = metadata.IndexDataSize;

		Asset asset;
		memcpy_s(asset.Identifier, IDENTIFIER_LENGTH, MODEL_ASSET_IDENTIFIER, IDENTIFIER_LENGTH);
		asset.Version = MODEL_ASSET_VERSION;

		const size_t packedSize = metadata.VertexDataSize + metadata.IndexDataSize;
		std::vector<char> packedData;
		packedData.resize(packedSize);

		memcpy_s(packedData.data(), metadata.VertexDataSize, vertices, metadata.VertexDataSize);
		memcpy_s(packedData.data() + metadata.VertexDataSize, metadata.IndexDataSize, indices, metadata.IndexDataSize);

		asset.Blob = CompressData(metadata.CompressionMode, { packedData.data(), packedData.size()});
		asset.Metadata = metadataJson.dump();
		return asset;
	}

	std::vector<char> UnpackVertex(const MeshMetadata& metadata, const std::span<const char> uncompressedData)
	{
		std::vector<char> vertices;
		vertices.resize(metadata.VertexSize);
		memcpy(vertices.data(), uncompressedData.data() + metadata.VertexOffset, metadata.VertexSize);
		return vertices;
	}

	std::vector<ModelIndexType> UnpackIndex(const ModelMetadata& modelMetadata, const MeshMetadata& metadata, std::span<const char> uncompressedData)
	{
		std::vector<ModelIndexType> indices;
		indices.resize(metadata.IndexSize);
		memcpy(indices.data(), uncompressedData.data() + modelMetadata.VertexDataSize + metadata.IndexOffset, metadata.IndexSize);
		return indices;
	}

	bool ConvertModel(const fs::path& input)
	{
		fs::path newOutputPath = input;
		newOutputPath.replace_extension(magic_enum::enum_name<EAssetExtension>(EAssetExtension::MODEL));
		return ConvertModel(input, newOutputPath);
	}

	MeshMetadata ProcessStaticMesh(const aiScene& scene, const aiMesh& mesh, std::vector<ModelVertexType>& vertices, std::vector<ModelIndexType>& indices)
	{
		MeshMetadata metadata;

		const size_t vertexOffset = SizeOfVertices(vertices.size());
		const size_t indexOffset = SizeOfIndices(indices.size());
		for (unsigned int idx = 0; idx < mesh.mNumVertices; ++idx)
		{
			const aiVector3D v = mesh.mVertices[idx];
			const aiVector3D t = mesh.HasTextureCoords(0) ? mesh.mTextureCoords[0][idx] : aiVector3D{ 0.f, 0.f, 0.f };
			const aiVector3D n = mesh.mNormals[idx];
			vertices.emplace_back(glm::vec3(v.x, v.z, v.y), glm::vec2(t.x, t.y), glm::vec3(n.x, n.y, n.z));
		}

		for (unsigned int idx = 0; idx < mesh.mNumFaces; ++idx)
		{
			for (int j = 0; j != 3; ++j)
			{
				indices.push_back(mesh.mFaces[idx].mIndices[j]);
			}
		}

		return { mesh.mName.C_Str(), vertexOffset, mesh.mNumVertices * sizeof(ModelVertexType), indexOffset, mesh.mNumFaces * 3 * sizeof(ModelIndexType) };
	}

	std::vector<MeshMetadata> ProcessScene(const aiScene& scene, std::vector<ModelVertexType>& vertices, std::vector<ModelIndexType>& indices)
	{
		std::vector<MeshMetadata> result;
		result.reserve(scene.mNumMeshes);

		size_t numOfVertices = 0;
		size_t numOfIndices = 0;
		for (size_t meshIdx = 0; meshIdx < scene.mNumMeshes; ++meshIdx)
		{
			numOfVertices += scene.mMeshes[meshIdx]->mNumVertices;
			numOfIndices += scene.mMeshes[meshIdx]->mNumFaces * 3;
		}
		vertices.reserve(numOfVertices);
		indices.reserve(numOfIndices);

		for (size_t meshIdx = 0; meshIdx < scene.mNumMeshes; ++meshIdx)
		{
			result.emplace_back(ProcessStaticMesh(scene, *scene.mMeshes[meshIdx], vertices, indices));
		}

		return result;
	}

	bool ConvertModel(const fs::path& input, const fs::path& output)
	{
		const auto inputPathStr = input.string();
		auto inputExtension = input.extension().string();
		if (inputExtension.size() < 2)
		{
			SY_ASSERT(false, "Invalid input extension length.");
			return false;
		}

		inputExtension = inputExtension.substr(1);
		std::transform(inputExtension.begin(), inputExtension.end(), inputExtension.begin(), ::toupper);

		const auto srcExtension = magic_enum::enum_cast<EModelExtension>(inputExtension);
		if (!srcExtension.has_value())
		{
			SY_ASSERT(false, "Invalid input mesh extension {}.", inputExtension);
			return false;
		}

		/** @TODO Impl convert mesh */
		constexpr unsigned int importFlags = aiProcess_GenNormals | aiProcess_GenUVCoords | aiProcess_GenBoundingBoxes | aiProcess_Triangulate | aiProcess_PreTransformVertices | aiProcess_FlipWindingOrder | aiProcess_FlipUVs;
		//constexpr unsigned int importFlags = aiProcess_GenNormals | aiProcess_GenUVCoords | aiProcess_GenBoundingBoxes | aiProcess_Triangulate | aiProcess_PreTransformVertices | aiProcess_FlipWindingOrder;
		const aiScene* scene = aiImportFile(input.string().c_str(), importFlags);
		if (scene == nullptr || !scene->HasMeshes())
		{
			SY_ASSERT(false, "Unable to load scene from {}.", input.string());
			return false;
		}

		ModelMetadata metadata;
		/** @todo: material import */
		std::vector<ModelVertexType> vertices;
		std::vector<ModelIndexType> indices;
		metadata.MeshMetadataList = ProcessScene(*scene, vertices, indices);
		metadata.VertexDataSize = SizeOfVertices(vertices.size());
		metadata.IndexDataSize = SizeOfIndices(indices.size());

		const auto newAssetOpt = Pack(metadata, vertices.data(), indices.data());
		if (!newAssetOpt.has_value())
		{
			SY_ASSERT(false, "Failed to packing mesh {}.", inputPathStr);
			return false;
		}

		aiReleaseImport(scene);
		SaveBinary(output.string(), newAssetOpt.value());
		return true;
	}

	std::vector<std::unique_ptr<render::Mesh>> LoadMeshesFromModelAsset(const std::string_view assetPath,
		const vk::VulkanContext& vulkanContext, vk::CommandPoolManager& cmdPoolManager,
		const vk::FrameTracker& frameTracker)
	{
		auto loadedAssetOpt = LoadBinary(assetPath, MODEL_ASSET_VERSION);
		if (!loadedAssetOpt.has_value())
		{
			SY_ASSERT(false, "Failed to load model asset from {}.", assetPath);
			return {};
		}

		auto& loadedAsset = loadedAssetOpt.value();
		const auto metadataOpt = ParseModelMetadata(loadedAsset);
		if (!metadataOpt.has_value())
		{
			SY_ASSERT(false, "Failed to parse metadata from {}.", assetPath);
			return {};
		}

		const auto& metadata = metadataOpt.value();
		const std::vector<char> uncompressedData = Uncompress(metadata.CompressionMode, metadata.VertexDataSize + metadata.IndexDataSize, loadedAsset.Blob);

		std::vector<std::unique_ptr<render::Mesh>> meshes;
		for (const auto& meshMetadata : metadata.MeshMetadataList)
		{
			const std::vector<char> verticesBytes = UnpackVertex(meshMetadata, uncompressedData);
			const std::vector<ModelIndexType> indices = UnpackIndex(metadata, meshMetadata, uncompressedData);

			meshes.emplace_back(render::Mesh::Create<ModelVertexType, ModelIndexType>(
				std::format("{}_{}", assetPath, meshMetadata.Name),
				vulkanContext, cmdPoolManager, frameTracker,
				std::span{ reinterpret_cast<const ModelVertexType*>(verticesBytes.data()), verticesBytes.size() / sizeof(ModelVertexType) },
				indices));
		}

		return meshes;
	}
}
