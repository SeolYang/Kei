#pragma once
#include <Core/Core.h>
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

	enum class EMeshType
	{
		Static,
		Skeletal /** @todo not impl yet! Bones, Animations, etc... */
	};

	struct MeshMetadata
	{
		EMeshType MeshType = EMeshType::Static;
		uint32_t Vertices = 0;
		uint32_t Indices = 0;
		std::string SrcPath;

		ECompressionMode CompressionMode = ECompressionMode::LZ4;
	};

	constexpr auto MeshTypeToVertexType(const EMeshType meshType)
	{
		switch (meshType)
		{
		case EMeshType::Static:
			return render::EVertexType::VertexPTN;

		default:
			return render::EVertexType::VertexPT;
		}
	}

	inline size_t CalculateVertexDataSizeFromMetadata(const MeshMetadata& metadata)
	{
		return render::SizeOfVertex(MeshTypeToVertexType(metadata.MeshType)) * metadata.Vertices;
	}

	inline size_t CalculateIndexDataSizeFromMetadata(const MeshMetadata& metadata)
	{
		return sizeof(render::IndexType) * metadata.Indices;
	}

	inline bool CheckMeshCompressionSupport(const ECompressionMode compression)
	{
		static robin_hood::unordered_set<ECompressionMode> SupportedCompression
		{
			ECompressionMode::LZ4
		};

		return SupportedCompression.find(compression) != SupportedCompression.end();
	}

	std::optional<MeshMetadata> ParseMeshMetadata(const Asset& asset);
	std::optional<Asset> Pack(const MeshMetadata& metadata, const void* vertices, const uint32_t* indices);
	std::vector<char> UnpackVertex(const MeshMetadata& metadata, std::span<const char> uncompressedData);
	std::vector<render::IndexType> UnpackIndex(const MeshMetadata& metadata, std::span<const char> uncompressedData);

	bool ConvertMesh(const fs::path& input);
	bool ConvertMesh(const fs::path& input, const fs::path& output);

	std::unique_ptr<render::Mesh> LoadMeshFromAsset(std::string_view assetPath, const vk::VulkanContext& vulkanContext, vk::CommandPoolManager& cmdPoolManager, const vk::FrameTracker& frameTracker);
}