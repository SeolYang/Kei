#include <Core/Core.h>
#include <Asset/MeshAsset.h>
#include <VK/Buffer.h>

namespace sy::asset
{

	constexpr std::string_view MESH_METADATA_MESH_TYPE = "MeshType";
	constexpr std::string_view MESH_METADATA_VERTICES = "Vertices";
	constexpr std::string_view MESH_METADATA_INDICES = "Indices";
	constexpr std::string_view MESH_METADATA_SRC_PATH = "SrcPath";
	constexpr std::string_view MESH_METADATA_COMPRESSION_MODE = "Compression";
	constexpr uint32_t MESH_ASSET_VERSION = 0;

	std::optional<MeshMetadata> ParseMeshMetadata(const Asset& asset)
	{
		MeshMetadata metadata;

		auto metadataJson = nlohmann::json::parse(asset.Metadata);

		const std::string meshTypeStr = metadataJson[MESH_METADATA_MESH_TYPE];
		const auto meshTypeOpt = magic_enum::enum_cast<EMeshType>(meshTypeStr);
		if (!meshTypeOpt.has_value())
		{
			SY_ASSERT(false, "Found invalid mesh type from mesh asset metadata.");
			return std::nullopt;
		}
		metadata.MeshType = meshTypeOpt.value();

		metadata.Vertices = metadataJson[MESH_METADATA_VERTICES];
		metadata.Indices = metadataJson[MESH_METADATA_INDICES];
		metadata.SrcPath = metadataJson[MESH_METADATA_SRC_PATH];

		const std::string compressionStr = metadataJson[MESH_METADATA_COMPRESSION_MODE];
		const auto compressionOpt = magic_enum::enum_cast<ECompressionMode>(compressionStr);
		if (!compressionOpt.has_value())
		{
			SY_ASSERT(false, "Found invalid mesh compression mode from asset metadata.");
			return std::nullopt;
		}
		metadata.CompressionMode = compressionOpt.value();

		if (!CheckMeshCompressionSupport(metadata.CompressionMode))
		{
			SY_ASSERT(false, "Unsupported compression mode for mesh asset.");
			return std::nullopt;
		}

		return metadata;
	}

	std::optional<Asset> Pack(const MeshMetadata& metadata, const void* vertices, const uint32_t* indices)
	{
		/** @todo mesh optimization options */
		if (vertices == nullptr)
		{
			SY_ASSERT(false, "Invalid vertex data.");
			return std::nullopt;
		}

		if (indices == nullptr)
		{
			SY_ASSERT(false, "Invalid index data.");
		}

		if (metadata.Vertices == 0)
		{
			spdlog::warn("Zero-vertex mesh asset packing.");
		}

		nlohmann::json metadataJson;
		metadataJson[MESH_METADATA_MESH_TYPE] = magic_enum::enum_name<EMeshType>(metadata.MeshType);
		metadataJson[MESH_METADATA_VERTICES] = metadata.Vertices;
		metadataJson[MESH_METADATA_INDICES] = metadata.Indices;
		metadataJson[MESH_METADATA_SRC_PATH] = metadata.SrcPath;

		if (!CheckMeshCompressionSupport(metadata.CompressionMode))
		{
			SY_ASSERT(false, "Unsupported compression mode for mesh asset.");
			return std::nullopt;
		}
		metadataJson[MESH_METADATA_COMPRESSION_MODE] = magic_enum::enum_name<ECompressionMode>(metadata.CompressionMode);

		Asset asset;
		memcpy_s(asset.Identifier, IDENTIFIER_LENGTH, MESH_ASSET_IDENTIFIER, IDENTIFIER_LENGTH);
		asset.Version = MESH_ASSET_VERSION;

		const size_t vertexDataSize = CalculateVertexDataSizeFromMetadata(metadata);
		const size_t indexDataSize = CalculateIndexDataSizeFromMetadata(metadata);
		const size_t packedDataSize = vertexDataSize + indexDataSize;
		std::vector<char> packedData;
		packedData.resize(packedDataSize);

		memcpy_s(packedData.data(), vertexDataSize, vertices, vertexDataSize);
		memcpy_s(packedData.data() + vertexDataSize, indexDataSize, indices, indexDataSize);

		asset.Blob = CompressData(metadata.CompressionMode, { packedData.data(), packedData.size()});
		asset.Metadata = metadataJson.dump();
		return asset;
	}

	std::vector<char> UnpackVertex(const MeshMetadata& metadata, std::span<const char> uncompressedData)
	{
		const size_t vertexDataSize = CalculateVertexDataSizeFromMetadata(metadata);
		std::vector<char> vertices;
		vertices.resize(vertexDataSize);
		memcpy(vertices.data(), uncompressedData.data(), vertexDataSize);
		return vertices;
	}

	std::vector<render::IndexType> UnpackIndex(const MeshMetadata& metadata, std::span<const char> uncompressedData)
	{
		const size_t indexDataOffset = CalculateVertexDataSizeFromMetadata(metadata);
		const size_t indexDataSize = CalculateIndexDataSizeFromMetadata(metadata);
		std::vector<render::IndexType> indices;
		indices.resize(metadata.Indices);
		memcpy(indices.data(), uncompressedData.data() + indexDataOffset, indexDataSize);
		return indices;
	}

	bool ConvertMesh(const fs::path& input)
	{
		fs::path newOutputPath = input;
		newOutputPath.replace_extension(magic_enum::enum_name<EAssetExtension>(EAssetExtension::MESH));
		return ConvertMesh(input, newOutputPath);
	}

	size_t ProcessStaticMesh(const aiScene& scene, std::vector<char>& vertices, std::vector<render::IndexType>& indices)
	{
		std::vector<render::VertexPTN> mergedVertices;

		for (size_t meshIdx = 0; meshIdx < scene.mNumMeshes; ++meshIdx)
		{
			const aiMesh& mesh = *scene.mMeshes[meshIdx];
			for (unsigned int idx = 0; idx < mesh.mNumVertices; ++idx)
			{
				const aiVector3D v = mesh.mVertices[idx];
				const aiVector3D t = mesh.mTextureCoords[0][idx];
				const aiVector3D n = mesh.mNormals[idx];
				mergedVertices.emplace_back(glm::vec3(v.x, v.z, v.y), glm::vec2(t.x, t.y), glm::vec3(n.x, n.y, n.z));
			}

			for (unsigned int idx = 0; idx < mesh.mNumFaces; ++idx)
			{
				for (int j = 0; j != 3; ++j)
				{
					indices.push_back(mesh.mFaces[idx].mIndices[j]);
				}
			}
		}

		vertices = ToBytes<render::VertexPTN>(mergedVertices);
		return mergedVertices.size();
	}

	size_t ProcessSkeletalMesh(const aiScene& scene, std::vector<char>& vertices, std::vector<render::IndexType>& indices)
	{
		SY_ASSERT(false, "Not implemented");
		return 0;
	}

	bool ConvertMesh(const fs::path& input, const fs::path& output)
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

		const auto srcExtension = magic_enum::enum_cast<EMeshExtension>(inputExtension);
		if (!srcExtension.has_value())
		{
			SY_ASSERT(false, "Invalid input mesh extension {}.", inputExtension);
			return false;
		}

		/** @TODO Impl convert mesh */
		constexpr unsigned int importFlags = aiProcess_GenNormals | aiProcess_GenUVCoords | aiProcess_GenBoundingBoxes | aiProcess_Triangulate;
		const aiScene* scene = aiImportFile(input.string().c_str(), importFlags);
		if (scene == nullptr || !scene->HasMeshes())
		{
			SY_ASSERT(false, "Unable to load scene from {}.", input.string());
			return false;
		}

		MeshMetadata metadata;
		metadata.SrcPath = inputPathStr;

		/** @todo: material import */
		std::vector<char> vertices;
		std::vector<render::IndexType> indices;
		if (scene->hasSkeletons())
		{
			// @todo additional data for animations?
			// @todo impl for skeletal mesh
			metadata.Vertices = ProcessStaticMesh(*scene, vertices, indices);
			metadata.MeshType = EMeshType::Static;
			//ProcessSkeletalMesh(*scene, vertices, indices);
			//metadata.MeshType = EMeshType::Skeletal;
		}
		else
		{
			metadata.Vertices = ProcessStaticMesh(*scene, vertices, indices);
			metadata.MeshType = EMeshType::Static;
		}

		metadata.Indices = indices.size();

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

	std::pair<std::unique_ptr<vk::Buffer>, std::unique_ptr<vk::Buffer>> LoadMeshFromAsset(std::string_view assetPath,
		const vk::VulkanContext& vulkanContext, vk::CommandPoolManager& cmdPoolManager,
		const vk::FrameTracker& frameTracker)
	{
		auto loadedAssetOpt = LoadBinary(assetPath, MESH_ASSET_VERSION);
		if (!loadedAssetOpt.has_value())
		{
			SY_ASSERT(false, "Failed to load model asset from {}.", assetPath);
			return { nullptr, nullptr };
		}

		auto& loadedAsset = loadedAssetOpt.value();
		const auto metadataOpt = ParseMeshMetadata(loadedAsset);
		if (!metadataOpt.has_value())
		{
			SY_ASSERT(false, "Failed to parse metadata from {}.", assetPath);
			return { nullptr, nullptr };
		}

		const auto& metadata = metadataOpt.value();
		const std::vector<char> uncompressedData = Uncompress(metadata.CompressionMode, CalculateVertexDataSizeFromMetadata(metadata) + CalculateIndexDataSizeFromMetadata(metadata), loadedAsset.Blob);
		const std::vector<char> vertices = UnpackVertex(metadata, uncompressedData);
		const std::vector<render::IndexType> indices = UnpackIndex(metadata, uncompressedData);

		return {
			vk::Buffer::CreateBufferWithData(cmdPoolManager, frameTracker, std::format("{}_Vertex", assetPath), vulkanContext, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, vertices.size(), vertices.data()),
			vk::Buffer::CreateIndexBuffer(cmdPoolManager, frameTracker, std::format("{}_Index", assetPath), vulkanContext, indices)
		};
	}
}
