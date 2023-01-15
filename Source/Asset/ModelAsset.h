#pragma once
#include <PCH.h>
#include <Asset/AssetCore.h>
#include <Render/Mesh.h>

/**
 * Mesh Asset structure
 * 1. Asset metadata
 * 2. Mesh metadata
 * 3. Compressed Vertex data
 * 4. Compressed Index data
 */

namespace sy::vk
{
	class VulkanContext;
	class CommandPoolManager;
	class FrameTracker;
}

namespace sy::asset
{
	struct MeshMetadata
	{
		std::string Name;
		size_t VertexOffset;
		size_t VertexSize;
		size_t IndexOffset;
		size_t IndexSize;
	};

	struct ModelMetadata
	{
		ECompressionMode CompressionMode = ECompressionMode::LZ4;
		std::vector<MeshMetadata> MeshMetadataList;
		size_t VertexDataSize;
		size_t IndexDataSize;
	};

	using ModelVertexType = render::VertexPTN;
	using ModelIndexType = render::IndexType;
	constexpr size_t SizeOfVertices(const size_t numOfVertices = 1)
	{
		return sizeof(ModelVertexType) * numOfVertices;
	}

	constexpr size_t SizeOfIndices(const size_t numOfIndices = 1)
	{
		return sizeof(ModelIndexType) * numOfIndices;
	}

	inline bool CheckModelCompressionSupport(const ECompressionMode compression)
	{
		static robin_hood::unordered_set<ECompressionMode> SupportedCompression
		{
			ECompressionMode::LZ4,
			ECompressionMode::None
		};

		return SupportedCompression.find(compression) != SupportedCompression.end();
	}

	std::optional<ModelMetadata> ParseModelMetadata(const Asset& asset);
	std::optional<Asset> Pack(const ModelMetadata& metadata, const void* vertices, const uint32_t* indices);
	std::vector<char> UnpackVertex(const MeshMetadata& metadata, std::span<const char> uncompressedData);
	std::vector<ModelIndexType> UnpackIndex(const ModelMetadata& modelMetadata, const MeshMetadata& metadata, std::span<const char> uncompressedData);

	bool ConvertModel(const fs::path& input);
	bool ConvertModel(const fs::path& input, const fs::path& output);

	std::vector<std::unique_ptr<render::Mesh>> LoadMeshesFromModelAsset(std::string_view assetPath, const vk::VulkanContext& vulkanContext, vk::CommandPoolManager& cmdPoolManager, const vk::FrameTracker& frameTracker);
}