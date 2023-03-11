#include <PCH.h>
#include <Asset/MaterialAsset.h>
#include <Asset/TextureAsset.h>
#include <Render/Material.h>
#include <VK/Texture.h>
#include <VK/TextureView.h>
#include <VK/Sampler.h>
#include <VK/VulkanContext.h>
#include <VK/DescriptorManager.h>

namespace sy::asset
{
	struct MaterialMetadata
	{
		std::string BaseTexture = vk::DefaultWhiteTexture.data();
		/// ...
	};

	constexpr std::string_view MATERIAL_METADATA_BASE_TEXTURE = "BaseTexture";

	MaterialMetadata QueryMetadata(const AssetData<render::Material>& assetData)
	{
		const nlohmann::json& metadataJson = assetData.GetMetadata();
		MaterialMetadata result;

		result.BaseTexture = metadataJson[MATERIAL_METADATA_BASE_TEXTURE];
		return result;
	}

	Handle<render::Material> LoadMaterialFromAsset(
		const fs::path& path,
		ResourceCache& resourceCache, const vk::VulkanContext& vulkanContext)
	{
		std::string pathStr = path.string();
		if (const auto handle = resourceCache.QueryAlias<render::Material>(pathStr); handle)
		{
			return handle;
		}

		const auto assetDataHandle = LoadOrCreateAssetData<render::Material>(path, resourceCache);
		if (!assetDataHandle)
		{
			SY_ASSERT(false, " Failed to load material asset from {}.", pathStr);
			return {};
		}

		const auto& vulkanRHI = vulkanContext.GetVulkanRHI();
		auto& descriptorManager = vulkanContext.GetDescriptorManager();
		const auto& assetData = Unwrap(resourceCache.Load(assetDataHandle));
		const auto metadata = QueryMetadata(assetData);

		const auto baseTexHandle = LoadTexture2DFromAsset(metadata.BaseTexture, resourceCache, vulkanContext);
		auto& baseTexRef = Unwrap(resourceCache.Load(baseTexHandle));
		const auto baseTexViewHandle = resourceCache.Add<vk::TextureView>(std::format("{}_View", metadata.BaseTexture), vulkanRHI, baseTexRef, VK_IMAGE_VIEW_TYPE_2D);
		const auto linearSampler = resourceCache.QueryAlias<vk::Sampler>(vk::LinearSamplerRepeat);
		const auto newHandle = resourceCache.Add<render::Material>(render::Material{ resourceCache.Add<vk::Descriptor>(descriptorManager.RequestDescriptor(resourceCache, baseTexHandle, baseTexViewHandle, linearSampler, vk::ETextureState::AnyShaderReadSampledImage)) });
		resourceCache.SetAlias(pathStr, newHandle);
		return newHandle;
	}

	nlohmann::json ToMetadata(const MaterialMetadata metadata)
	{
		nlohmann::json result;
		result[MATERIAL_METADATA_BASE_TEXTURE] = metadata.BaseTexture;
		return result;
	}

	void CreateMaterial(const fs::path& path)
	{
		fs::create_directory(path.parent_path());
		const MaterialMetadata metadata;
		const AssetData<render::Material> assetData{ path, ToMetadata(metadata), {} };
		assetData.SaveMetadata();
	}
}
