#include <Core/Core.h>
#include <Asset/TextureAsset.h>
#include <VK/Texture.h>

namespace sy::asset
{
	constexpr std::string_view TEXTURE_METADATA_FORMAT = "Format";
	constexpr std::string_view TEXTURE_METADATA_COMPRESSION_MODE = "Compression";
	constexpr std::string_view TEXTURE_METADATA_WIDTH = "Width";
	constexpr std::string_view TEXTURE_METADATA_HEIGHT = "Height";
	constexpr std::string_view TEXTURE_METADATA_DEPTH = "Depth";
	constexpr std::string_view TEXTURE_METADATA_BUFFER_SIZE = "BufferSize";
	constexpr std::string_view TEXTURE_METADATA_SRC_PATH = "SrcPath";
	constexpr uint32_t TEXTURE_ASSET_VERSION = 0;

	std::optional<TextureMetadata> ParseTextureMetadata(const Asset& asset)
	{
		TextureMetadata metadata;

		auto metadataJson = nlohmann::json::parse(asset.Metadata);

		const std::string textureFormatStr = metadataJson[TEXTURE_METADATA_FORMAT];
		const auto formatOpt = magic_enum::enum_cast<VkFormat>(textureFormatStr);
		if (!formatOpt.has_value())
		{
			SY_ASSERT(false, "Found invalid texture format from asset metadata.");
			return std::nullopt;
		}
		metadata.Format = formatOpt.value();

		const std::string compressionStr = metadataJson[TEXTURE_METADATA_COMPRESSION_MODE];
		const auto compressionOpt = magic_enum::enum_cast<ECompressionMode>(compressionStr);
		if (!compressionOpt.has_value())
		{
			SY_ASSERT(false, "Found invalid texture compression mode from asset metadata.");
			return std::nullopt;
		}
		metadata.CompressionMode = compressionOpt.value();
		if (!CheckTextureCompressionSupport(metadata.CompressionMode))
		{
			SY_ASSERT(false, "Unsupported compression mode for texture asset.");
			return std::nullopt;
		}

		metadata.Extent.width = metadataJson[TEXTURE_METADATA_WIDTH];
		metadata.Extent.height = metadataJson[TEXTURE_METADATA_HEIGHT];
		metadata.Extent.depth = metadataJson[TEXTURE_METADATA_DEPTH];
		metadata.BufferSize = metadataJson[TEXTURE_METADATA_BUFFER_SIZE];
		metadata.SrcPath = metadataJson[TEXTURE_METADATA_SRC_PATH];

		return metadata;
	}

	std::optional<Asset> Pack(const TextureMetadata& metadata, const void* pixelData)
	{
		if (pixelData == nullptr)
		{
			SY_ASSERT(false, "Invalid pixel data.");
			return std::nullopt;
		}

		if (const bool bInvalidExtent = metadata.Extent.width < 1 || metadata.Extent.height < 1 || metadata.Extent.depth < 1;
			bInvalidExtent)
		{
			SY_ASSERT(false, "Invalid texture extent");
			return std::nullopt;
		}

		if (const bool bInvalidBufferSize = metadata.BufferSize < 1;
			bInvalidBufferSize)
		{
			SY_ASSERT(false, "Invalid buffer size.");
			return std::nullopt;
		}

		nlohmann::json metadataJson;
		metadataJson[TEXTURE_METADATA_FORMAT] = magic_enum::enum_name<VkFormat>(metadata.Format);
		metadataJson[TEXTURE_METADATA_WIDTH] = metadata.Extent.width;
		metadataJson[TEXTURE_METADATA_HEIGHT] = metadata.Extent.height;
		metadataJson[TEXTURE_METADATA_DEPTH] = metadata.Extent.depth;
		metadataJson[TEXTURE_METADATA_BUFFER_SIZE] = metadata.BufferSize;
		metadataJson[TEXTURE_METADATA_SRC_PATH] = metadata.SrcPath;

		Asset asset;
		memcpy_s(asset.Identifier, IDENTIFIER_LENGTH, TEXTURE_ASSET_IDENTIFIER, IDENTIFIER_LENGTH);
		asset.Version = TEXTURE_ASSET_VERSION;

		if (!CheckTextureCompressionSupport(metadata.CompressionMode))
		{
			SY_ASSERT(false, "Unsupported Compression mode for texture asset.");
			return std::nullopt;
		}
		metadataJson[TEXTURE_METADATA_COMPRESSION_MODE] = magic_enum::enum_name<ECompressionMode>(metadata.CompressionMode);

		asset.Blob = CompressData(metadata.CompressionMode, {static_cast<const char*>(pixelData), metadata.BufferSize });
		asset.Metadata = metadataJson.dump();
		return asset;
	}

	std::vector<char> Unpack(const TextureMetadata& metadata, std::span<const char> src)
	{
		return Uncompress(metadata.CompressionMode, metadata.BufferSize, src);
	}

	bool ConvertTexture2D(const fs::path& input)
	{
		fs::path newOutputPath = input;
		newOutputPath.replace_extension(magic_enum::enum_name(EAssetExtension::TEX));
		return ConvertTexture2D(input, newOutputPath);
	}

	bool ConvertTexture2D(const fs::path& input, const fs::path& output)
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

		const auto srcExtension = magic_enum::enum_cast<ETextureExtension>(inputExtension);
		if (!srcExtension.has_value())
		{
			SY_ASSERT(false, "Invalid input texture extension {}.", inputExtension);
				return false;
		}

		const auto requiredFormat = ExtensionToFormat(srcExtension.value());
		const auto requiredChannels = vk::ToNumberOfComponents(requiredFormat);

		int width, height, channels;
		stbi_uc* pixels = stbi_load(inputPathStr.c_str(), &width, &height, &channels, requiredChannels);
		if (pixels == nullptr)
		{
			SY_ASSERT(false, "Failed load data from file {}.", input.string());
			return false;
		}

		const auto requiredSize = vk::ToByteSize(requiredFormat) * width * height;

		TextureMetadata metadata;
		metadata.BufferSize = requiredSize;
		metadata.Extent = Extent3D<uint32_t>{ static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1 };
		metadata.Format = requiredFormat;
		metadata.SrcPath = inputPathStr;

		const auto newAssetOpt = Pack(metadata, pixels);
		if (!newAssetOpt.has_value())
		{
			SY_ASSERT(false, "Failed to packing texture {}.", inputPathStr);
			return false;
		}

		stbi_image_free(pixels);

		SaveBinary(output.string(), newAssetOpt.value());
		return true;
	}

	std::unique_ptr<vk::Texture2D> LoadTextureFromAsset(std::string_view assetPath,
		const vk::VulkanContext& vulkanContext, const vk::FrameTracker& frameTracker,
		vk::CommandPoolManager& cmdPoolManager)
	{
		auto loadedAssetOpt = asset::LoadBinary(assetPath, TEXTURE_ASSET_VERSION);
		if (!loadedAssetOpt.has_value())
		{
			SY_ASSERT(false, "Failed to load texture 2d asset from {}.", assetPath);
			return nullptr;
		}

		auto& loadedAsset = loadedAssetOpt.value();
		const auto metadataOpt = asset::ParseTextureMetadata(loadedAsset);
		if (!metadataOpt.has_value())
		{
			SY_ASSERT(false, "Failed to parse metadata from {}.", assetPath);
			return nullptr;
		}

		const auto& metadata = metadataOpt.value();
		const std::vector<char> data = Unpack(metadata, loadedAsset.Blob);

		return vk::Texture2D::LoadFromMemory(
			assetPath,
			vulkanContext, frameTracker, cmdPoolManager,
			data,
			Extent2D<uint32_t>{metadata.Extent.width, metadata.Extent.height},
			metadata.Format);
	}
}
