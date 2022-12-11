#include <Core/Core.h>
#include <Asset/TextureAsset.h>
#include <VK/Texture.h>

namespace sy::asset
{
	std::optional<TextureMetadata> ParseMetadata(const Asset& asset)
	{
		TextureMetadata metadata;

		auto metadataJson = nlohmann::json::parse(asset.Metadata);

		const std::string textureFormatStr = metadataJson["Format"];
		const auto formatOpt = magic_enum::enum_cast<VkFormat>(textureFormatStr);
		if (!formatOpt.has_value())
		{
			spdlog::critical("Found invalid texture format from asset metadata.");
			return std::nullopt;
		}
		metadata.Format = formatOpt.value();

		const std::string compressionStr = metadataJson["Compression"];
		const auto compressionOpt = magic_enum::enum_cast<ECompressionMode>(compressionStr);
		if (!compressionOpt.has_value())
		{
			spdlog::critical("Found invalid texture compression mode from asset metadata.");
			return std::nullopt;
		}
		metadata.CompressionMode = compressionOpt.value();

		metadata.Extent.width = metadataJson["Width"];
		metadata.Extent.height = metadataJson["Height"];
		metadata.Extent.depth = metadataJson["Depth"];
		metadata.BufferSize = metadataJson["BufferSize"];
		metadata.SrcPath = metadataJson["SrcPath"];

		return metadata;
	}

	std::optional<Asset> Pack(TextureMetadata& metadata, const void* pixelData)
	{
		if (pixelData == nullptr)
		{
			spdlog::warn("Invalid pixel data.");
			return std::nullopt;
		}

		if (const bool bInvalidExtent = metadata.Extent.width < 1 || metadata.Extent.height < 1 || metadata.Extent.depth < 1;
			bInvalidExtent)
		{
			spdlog::warn("Invalid pixel extent.");
			return std::nullopt;
		}

		if (const bool bInvalidBufferSize = metadata.BufferSize < 1;
			bInvalidBufferSize)
		{
			spdlog::warn("Invalid buffer size.");
			return std::nullopt;
		}

		nlohmann::json metadataJson;
		metadataJson["Format"] = magic_enum::enum_name<VkFormat>(metadata.Format);
		metadataJson["Width"] = metadata.Extent.width;
		metadataJson["Height"] = metadata.Extent.height;
		metadataJson["Depth"] = metadata.Extent.depth;
		metadataJson["BufferSize"] = metadata.BufferSize;
		metadataJson["SrcPath"] = metadata.SrcPath;

		Asset asset;
		constexpr auto identifierLength = LengthOfArray(asset.Identifier);
		memcpy_s(asset.Identifier, identifierLength, TEXTURE_ASSET_IDENTIFIER, identifierLength);

		const auto compressStagingSize = LZ4_compressBound(static_cast<int>(metadata.BufferSize));
		asset.Blob.resize(compressStagingSize);

		const auto compressedSize = LZ4_compress_default(static_cast<const char*>(pixelData), asset.Blob.data(), static_cast<int>(metadata.BufferSize), compressStagingSize);
		asset.Blob.resize(compressedSize);
		asset.Blob.shrink_to_fit();

		metadataJson["Compression"] = magic_enum::enum_name<ECompressionMode>(metadata.CompressionMode);

		asset.Metadata = metadataJson.dump();

		return asset;
	}

	std::unique_ptr<vk::Texture2D> LoadTextureFromAsset(std::string_view assetPath,
		const vk::VulkanContext& vulkanContext, const vk::FrameTracker& frameTracker,
		vk::CommandPoolManager& cmdPoolManager)
	{
		auto loadedAssetOpt = asset::LoadBinary(assetPath);
		if (!loadedAssetOpt.has_value())
		{
			SY_ASSERT(false, "Failed to load texture 2d asset from {}.", assetPath);
			return nullptr;
		}

		auto& loadedAsset = loadedAssetOpt.value();
		const auto metadataOpt = asset::ParseMetadata(loadedAsset);
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

	std::vector<char> Unpack(const TextureMetadata& metadata, std::span<const char> src)
	{
		std::vector<char> dest;
		dest.resize(metadata.BufferSize);
		switch (metadata.CompressionMode)
		{
		case ECompressionMode::LZ4:
			LZ4_decompress_safe(src.data(), dest.data(), static_cast<int>(src.size()), static_cast<int>(dest.size()));
			break;

		default:
			memcpy(dest.data(), src.data(), src.size());
			break;
		}

		return dest;
	}

	VkFormat ExtensionToFormat(const ETextureExtension extension)
	{
		static const robin_hood::unordered_map<ETextureExtension, VkFormat> Table
		{
				{ ETextureExtension::HDR,		VK_FORMAT_R32G32B32_SFLOAT		},
				{ ETextureExtension::PNG,		VK_FORMAT_R8G8B8A8_SRGB			},
				{ ETextureExtension::JPEG,		VK_FORMAT_R8G8B8A8_SRGB			},
				{ ETextureExtension::JPG,		VK_FORMAT_R8G8B8A8_SRGB			}
		};

		return Table.find(extension)->second;
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
			SY_ASSERT(false, "Invalid input texture extension {}.", inputExtension)
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
			spdlog::warn("Failed to packing texture {}.", inputPathStr);
			return false;
		}

		stbi_image_free(pixels);

		SaveBinary(output.string(), newAssetOpt.value());
		spdlog::info("Convert texture resource {} to texture asset {}.", input.string(), output.string());
		return true;
	}
}
