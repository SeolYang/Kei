#include <Core/Core.h>
#include <Assets/TextureAsset.h>

namespace sy::asset::texture
{
	std::optional<Metadata> ParseTextureAssetInfo(Asset& asset)
	{
		Metadata metadata;

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

	std::optional<Asset> Pack(Metadata& metadata, const void* pixelData)
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

	std::vector<char> Unpack(const Metadata& metadata, std::span<const char> src)
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

	VkFormat FormatToExtension(const EExtension extension)
	{
		static const robin_hood::unordered_map<EExtension, VkFormat> Table
		{
				{ EExtension::HDR,		VK_FORMAT_R32G32B32_SFLOAT		},
				{ EExtension::PNG,		VK_FORMAT_R8G8B8A8_SRGB			},
				{ EExtension::JPEG,		VK_FORMAT_R8G8B8A8_SRGB			},
				{ EExtension::JPG,		VK_FORMAT_R8G8B8A8_SRGB			}
		};

		return Table.find(extension)->second;
	}
}
