#include <PCH.h>
#include <Asset/ModelAsset.h>
#include <Asset/MaterialAsset.h>
#include <VK/Buffer.h>
#include <Core/ResourceCache.h>
#include <Component/StaticMeshComponent.h>
#include <Render/Material.h>

namespace sy::asset
{
	struct MeshMetadata
	{
		std::string Name;
		size_t VerticesOffset;
		size_t NumVertices;
		size_t IndicesOffset;
		size_t NumIndices;
		std::string Material;
		std::string MaterialName;
	};

	struct ModelMetadata
	{
		std::vector<MeshMetadata> Meshes;
		size_t NumVertices;
		size_t NumIndices;
	};

	constexpr std::string_view MODEL_METADATA_MESHES = "Meshes";
	constexpr std::string_view MODEL_METADATA_NUM_VERTICES = "NumVertices";
	constexpr std::string_view MODEL_METADATA_NUM_INDICES = "NumIndices";
	constexpr std::string_view MESH_METADATA_NAME = "Name";
	constexpr std::string_view MESH_METADATA_VERTICES_OFFSET = "VerticesOffset";
	constexpr std::string_view MESH_METADATA_NUM_VERTICES = "NumVertices";
	constexpr std::string_view MESH_METADATA_INDICES_OFFSET = "IndicesOffset";
	constexpr std::string_view MESH_METADATA_NUM_INDICES = "NumIndices";
	constexpr std::string_view MESH_METADATA_MATERIAL = "Material";
	constexpr std::string_view MESH_METADATA_MATERIAL_NAME = "MaterialName";

	using ModelVertex = render::VertexPTN;
	using ModelIndex = render::IndexType;

	constexpr size_t SizeOfVertex = sizeof(ModelVertex);
	constexpr size_t SizeOfIndex = sizeof(ModelIndex);

	std::vector<MeshMetadata> QueryMeshes(const nlohmann::json& json)
	{
		const nlohmann::json& meshes = json[MODEL_METADATA_MESHES];
		std::vector<MeshMetadata> result;
		result.reserve(meshes.size());
		for (const auto& mesh : meshes)
		{
			result.emplace_back(
				mesh[MESH_METADATA_NAME],
				mesh[MESH_METADATA_VERTICES_OFFSET], mesh[MESH_METADATA_NUM_VERTICES],
				mesh[MESH_METADATA_INDICES_OFFSET], mesh[MESH_METADATA_NUM_INDICES],
				mesh[MESH_METADATA_MATERIAL]);
		}

		return result;
	}

	ModelMetadata QueryMetadata(const AssetData<render::Model>& assetData)
	{
		const nlohmann::json& metadataJson = assetData.GetMetadata();
		return {
			QueryMeshes(metadataJson),
			metadataJson[MODEL_METADATA_NUM_VERTICES],
			metadataJson[MODEL_METADATA_NUM_INDICES] };
	}

	std::vector<component::StaticMeshComponent> LoadModel(const std::string& name, const fs::path& path,
		ResourceCache& resourceCache,
		const vk::VulkanContext& vulkanContext, vk::CommandPoolManager& cmdPoolManager,
		const vk::FrameTracker& frameTracker, vk::DescriptorManager& descriptorManager)
	{
		const auto pathStr = path.string();
		const auto assetDataHandle = LoadOrCreateAssetData<render::Model>(path, resourceCache);
		if (!assetDataHandle)
		{
			SY_ASSERT(false, " Failed to load model asset from {}.", pathStr);
			return {};
		}

		const auto& assetData = Unwrap(resourceCache.Load(assetDataHandle));
		const auto metadata = QueryMetadata(assetData);
		const auto& blob = assetData.GetBlob();

		std::vector<ModelVertex> vertices;
		std::vector<ModelIndex> indices;
		vertices.resize(metadata.NumVertices);
		indices.resize(metadata.NumIndices);

		const size_t sizeOfVerticesBytes = SizeOfVertex * metadata.NumVertices;
		const size_t sizeOfIndicesBytes = SizeOfIndex * metadata.NumIndices;
		std::memcpy(vertices.data(), blob.data(), sizeOfVerticesBytes);
		std::memcpy(indices.data(), blob.data() + sizeOfVerticesBytes, sizeOfIndicesBytes);

		std::vector<component::StaticMeshComponent> components;
		for (const auto& mesh : metadata.Meshes)
		{
			/** @todo material load first! */
			const std::span verticesSpan{ vertices.data() + mesh.VerticesOffset, mesh.NumVertices };
			const std::span indicesSpan{ indices.data() + mesh.IndicesOffset, mesh.NumIndices };
			const Handle<render::Mesh> meshHandle = resourceCache.Add(render::Mesh::Create<ModelVertex, ModelIndex>(
				std::format("{}_{}", path.string(), mesh.Name),
				vulkanContext, cmdPoolManager, frameTracker,
				verticesSpan, indicesSpan));
			const Handle<render::Material> materialHandle = LoadMaterialFromAsset(mesh.Material, resourceCache, vulkanContext, frameTracker, cmdPoolManager, descriptorManager);
			component::StaticMeshComponent component;
			component.Mesh = meshHandle;
			component.Material = materialHandle;
			components.emplace_back(component);
		}

		return components;
	}

	auto PackMeshesToJson(const ModelMetadata& metadata)
	{
		nlohmann::json result;
		for (const auto& mesh : metadata.Meshes)
		{
			nlohmann::json meshJson;
			meshJson[MESH_METADATA_NAME] = mesh.Name;
			meshJson[MESH_METADATA_VERTICES_OFFSET] = mesh.VerticesOffset;
			meshJson[MESH_METADATA_NUM_VERTICES] = mesh.NumVertices;
			meshJson[MESH_METADATA_INDICES_OFFSET] = mesh.IndicesOffset;
			meshJson[MESH_METADATA_NUM_INDICES] = mesh.NumIndices;
			meshJson[MESH_METADATA_MATERIAL] = mesh.Material;
			meshJson[MESH_METADATA_MATERIAL_NAME] = mesh.MaterialName;
			result.push_back(meshJson);
		}

		return result;
	}

	auto PackMetadataToJson(const ModelMetadata& metadata)
	{
		nlohmann::json result;
		result[MODEL_METADATA_MESHES] = PackMeshesToJson(metadata);
		result[MODEL_METADATA_NUM_VERTICES] = metadata.NumVertices;
		result[MODEL_METADATA_NUM_INDICES] = metadata.NumIndices;
		return result;
	}

	MeshMetadata ProcessStaticMesh(const aiScene& scene, const aiMesh& mesh, std::vector<ModelVertex>& vertices, std::vector<ModelIndex>& indices)
	{
		MeshMetadata metadata;

		const size_t vertexOffset = vertices.size();
		const size_t indexOffset = indices.size();
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

		std::string materialName{};
		if (scene.HasMaterials())
		{
			const auto& material = *scene.mMaterials[mesh.mMaterialIndex];
			materialName = material.GetName().C_Str();
		}

		const std::string defaultMaterial = render::DefaultMaterial.data();
		return { mesh.mName.C_Str(), vertexOffset, mesh.mNumVertices, indexOffset, mesh.mNumFaces * 3, defaultMaterial, materialName };
	}

	std::vector<MeshMetadata> ProcessScene(const aiScene& scene, std::vector<ModelVertex>& vertices, std::vector<ModelIndex>& indices)
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

		/** @todo process materials */
		for (size_t materialIdx = 0; materialIdx < scene.mNumMaterials; ++materialIdx)
		{
			aiMaterial& material = *scene.mMaterials[materialIdx];
			spdlog::trace("Material Idx: {}, Name: {}, Texture Count: {}", materialIdx, material.GetName().C_Str(), material.GetTextureCount(aiTextureType_DIFFUSE));
		}

		for (size_t meshIdx = 0; meshIdx < scene.mNumMeshes; ++meshIdx)
		{
			result.emplace_back(ProcessStaticMesh(scene, *scene.mMeshes[meshIdx], vertices, indices));
		}

		return result;
	}

	bool ConvertModel(const fs::path& path)
	{
		fs::path outputPath = path;
		outputPath.replace_extension(magic_enum::enum_name(EAsset::Model));

		const std::string pathStr = path.string();
		constexpr unsigned int importFlags = aiProcess_GenNormals | aiProcess_GenUVCoords | aiProcess_GenBoundingBoxes | aiProcess_Triangulate | aiProcess_PreTransformVertices | aiProcess_FlipWindingOrder | aiProcess_FlipUVs;
		//constexpr unsigned int importFlags = aiProcess_GenNormals | aiProcess_GenUVCoords | aiProcess_GenBoundingBoxes | aiProcess_Triangulate | aiProcess_PreTransformVertices | aiProcess_FlipWindingOrder;
		const aiScene* scene = aiImportFile(pathStr.c_str(), importFlags);
		if (scene == nullptr || !scene->HasMeshes())
		{
			SY_ASSERT(false, "Unable to load scene from {}.", pathStr);
			return false;
		}

		ModelMetadata metadata;
		/** @todo: material import */
		std::vector<ModelVertex> vertices;
		std::vector<ModelIndex> indices;
		metadata.Meshes = ProcessScene(*scene, vertices, indices);
		metadata.NumVertices = vertices.size();
		metadata.NumIndices = indices.size();

		const size_t sizeOfVerticesBytes = metadata.NumVertices * SizeOfVertex;
		const size_t sizeOfIndicesBytes = metadata.NumIndices * SizeOfIndex;
		std::vector<char> blob;
		blob.resize(sizeOfVerticesBytes + sizeOfIndicesBytes);
		memcpy(blob.data(), vertices.data(), sizeOfVerticesBytes);
		memcpy(blob.data() + sizeOfVerticesBytes, indices.data(), sizeOfIndicesBytes);

		const auto newAssetData = AssetData<render::Model>(outputPath, PackMetadataToJson(metadata), std::move(blob));
		newAssetData.SaveBlob();
		newAssetData.SaveMetadata();
		return true;
	}
}