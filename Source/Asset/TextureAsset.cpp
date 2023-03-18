#include <PCH.h>
#include <Asset/TextureAsset.h>
#include <VK/Texture.h>
#include <VK/VulkanContext.h>
#include <VK/TextureBuilder.h>

namespace sy::asset
{
	struct TextureMetadata
	{
		VkFormat Format = VK_FORMAT_UNDEFINED;
		Extent3D<uint32_t> Extent = { 1, 1, 1 };
	};

	constexpr std::string_view TEXTURE_METADATA_FORMAT = "Format";
	constexpr std::string_view TEXTURE_METADATA_WIDTH = "Width";
	constexpr std::string_view TEXTURE_METADATA_HEIGHT = "Height";
	constexpr std::string_view TEXTURE_METADATA_DEPTH = "Depth";

	TextureMetadata QueryMetadata(const AssetData<vk::Texture>& assetData)
	{
		const nlohmann::json& metadataJson = assetData.GetMetadata();
		TextureMetadata result;

		const std::string formatStr = metadataJson[TEXTURE_METADATA_FORMAT];
		const auto formatOpt = magic_enum::enum_cast<VkFormat>(formatStr);
		result.Format = formatOpt.value_or(VK_FORMAT_UNDEFINED);
		result.Extent = {
			metadataJson[TEXTURE_METADATA_WIDTH],
			metadataJson[TEXTURE_METADATA_HEIGHT],
			metadataJson[TEXTURE_METADATA_DEPTH]
		};
		return result;
	}

	std::unique_ptr<vk::Texture> LoadTexture2DFromAsset(
		const std::string_view name,
		const AssetData<vk::Texture>& assetData,
		vk::VulkanContext& vulkanContext)
	{
		const auto& blob = assetData.GetBlob();
		const auto metadata = QueryMetadata(assetData);

		return vk::TextureBuilder::Texture2DShaderResourceTemplate(vulkanContext)
			.SetName(name)
			.SetExtent(metadata.Extent)
			.SetFormat(metadata.Format)
			.SetDataToTransfer(std::span{ blob })
			.Build();
	}

	Handle<vk::Texture> LoadTexture2DFromAsset(
		const Handle<AssetData<vk::Texture>> assetDataHandle,
		HandleManager& handleManager,
		vk::VulkanContext& vulkanContext)
	{
		if (!assetDataHandle)
		{
			return {};
		}

		const auto pathStr = assetDataHandle->GetPath().string();

		auto textureHandle = handleManager.QueryAlias<vk::Texture>(pathStr);
		if (textureHandle)
		{
			return textureHandle;
		}

		textureHandle = handleManager.Add(LoadTexture2DFromAsset(pathStr, *assetDataHandle, vulkanContext));
		textureHandle.SetAlias(pathStr);
		return textureHandle;
	}

	Handle<vk::Texture> LoadTexture2DFromAsset(const fs::path& path,
		HandleManager& handleManager, vk::VulkanContext& vulkanContext)
	{
		return LoadTexture2DFromAsset(LoadOrCreateAssetData<vk::Texture>(path, handleManager), handleManager,
			vulkanContext);
	}

	auto PackMetadataToJson(const TextureMetadata metadata)
	{
		nlohmann::json metadataJson;
		metadataJson[TEXTURE_METADATA_FORMAT] = magic_enum::enum_name(metadata.Format);
		metadataJson[TEXTURE_METADATA_WIDTH] = metadata.Extent.width;
		metadataJson[TEXTURE_METADATA_HEIGHT] = metadata.Extent.height;
		metadataJson[TEXTURE_METADATA_DEPTH] = metadata.Extent.depth;
		return metadataJson;
	}

	bool ConvertTexture2D(const fs::path& input)
	{
		fs::path outputPath = input;
		outputPath.replace_extension(magic_enum::enum_name(EAsset::Texture));

		std::string inputExtension = input.extension().string();
		inputExtension = inputExtension.substr(1);
		std::transform(inputExtension.begin(), inputExtension.end(), inputExtension.begin(), ::toupper);
		const auto srcExtension = magic_enum::enum_cast<ETextureExtension>(inputExtension);
		if (!srcExtension.has_value())
		{
			SY_ASSERT(false, "Invalid input texture extension {}.", inputExtension);
			return false;
		}

		const auto requiredFormat = ExtensionToFormat(srcExtension.value());
		const auto requiredChannels = vk::ToNumberOfComponents(requiredFormat);

		int width, height, channels;
		const std::string inputStr = input.string();
		stbi_uc* pixels = stbi_load(inputStr.c_str(), &width, &height, &channels, requiredChannels);
		if (pixels == nullptr)
		{
			SY_ASSERT(false, "Failed load data from file {}.", input.string());
			return false;
		}

		const auto requiredSize = vk::ToByteSize(requiredFormat) * width * height;
		TextureMetadata metadata;
		metadata.Extent = Extent3D<uint32_t>{ static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1 };
		metadata.Format = requiredFormat;

		std::vector<char> buffer;
		buffer.resize(requiredSize);
		std::memcpy(buffer.data(), pixels, requiredSize);
		stbi_image_free(pixels);

		const auto newAssetData = AssetData<vk::Texture>(outputPath, PackMetadataToJson(metadata), buffer);
		newAssetData.SaveBlob();
		newAssetData.SaveMetadata();
		return true;
	}
} // namespace sy::asset
