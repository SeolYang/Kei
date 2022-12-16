#include <Core/Core.h>
#include <Asset/MeshAsset.h>

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
		metadataJson[MESH_METADATA_SRC_PATH] = metadataJson["SrcPath"];

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

	std::vector<char> Uncompress(const MeshMetadata& metadata, std::span<const char> compressedData)
	{
		const size_t packedDataSize = CalculateVertexDataSizeFromMetadata(metadata) + CalculateIndexDataSizeFromMetadata(metadata);
		std::vector<char> uncompressedData;
		uncompressedData.resize(packedDataSize);

		switch (metadata.CompressionMode)
		{
		case ECompressionMode::LZ4:
			LZ4_decompress_safe(compressedData.data(), uncompressedData.data(), static_cast<int>(compressedData.size()), static_cast<int>(uncompressedData.size()));
			break;

		case ECompressionMode::None:
		default:
			memcpy(uncompressedData.data(), compressedData.data(), compressedData.size());
			break;
		}

		return uncompressedData;
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

	bool ConvertMesh(const fs::path& input, const fs::path& output)
	{
		/** @TODO Impl convert mesh */
		return false;
	}
}
