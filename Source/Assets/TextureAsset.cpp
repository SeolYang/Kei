#include <Core/Core.h>
#include <Assets/TextureAsset.h>

namespace sy
{
	namespace asset
	{
		std::optional<TextureInfo> ParseTextureAssetInfo(Asset& asset)
		{
			TextureInfo info;

			auto metadata = nlohmann::json::parse(asset.Metadata);

			const std::string textureFormatStr = metadata["Format"];
			const auto formatOpt =  magic_enum::enum_cast<ETextureFormat>(textureFormatStr);
			if (!formatOpt.has_value())
			{
				spdlog::critical("Found invalid texture format from asset metadata.");
				return std::nullopt;
			}
			info.Format = formatOpt.value();

			const std::string compressionStr = metadata["Compression"];
			const auto compressionOpt = magic_enum::enum_cast<ECompressionMode>(compressionStr);
			if (!compressionOpt.has_value())
			{
				spdlog::critical("Found invalid texture compression mode from asset metadata.");
				return std::nullopt;
			}
			info.CompressionMode = compressionOpt.value();

			info.Extent.width = metadata["Width"];
			info.Extent.height = metadata["Height"];
			info.Extent.depth = metadata["Depth"];
			info.BufferSize = metadata["BufferSize"];
			info.SrcPath = metadata["SrcPath"];

			return info;
		}

		std::optional<Asset> PackTexture(TextureInfo& info, const void* pixelData)
		{
			if (pixelData == nullptr)
			{
				spdlog::warn("Invalid pixel data.");
				return std::nullopt;
			}

			if (const bool bInvalidExtent = info.Extent.width < 1 || info.Extent.height < 1 || info.Extent.depth < 1; 
				bInvalidExtent)
			{
				spdlog::warn("Invalid pixel extent.");
				return std::nullopt;
			}

			if (const bool bInvalidBufferSize = info.BufferSize < 1;
				bInvalidBufferSize)
			{
				spdlog::warn("Invalid buffer size.");
				return std::nullopt;
			}

			nlohmann::json metadata;
			metadata["Format"] = magic_enum::enum_name<ETextureFormat>(info.Format);
			metadata["Width"] = info.Extent.width;
			metadata["Height"] = info.Extent.height;
			metadata["Depth"] = info.Extent.depth;
			metadata["BufferSize"] = info.BufferSize;
			metadata["SrcPath"] = info.SrcPath;

			Asset asset;
			constexpr auto identifierLength = LengthOfArray(asset.Identifier);
			memcpy_s(asset.Identifier, identifierLength, TEXTURE_ASSET_IDENTIFIER, identifierLength);

			const auto compressStagingSize = LZ4_compressBound(static_cast<int>(info.BufferSize));
			asset.Blob.resize(compressStagingSize);

			const auto compressedSize = LZ4_compress_default(static_cast<const char*>(pixelData), asset.Blob.data(), static_cast<int>(info.BufferSize), compressStagingSize);
			asset.Blob.resize(compressedSize);
			asset.Blob.shrink_to_fit();

			metadata["Compression"] = magic_enum::enum_name<ECompressionMode>(info.CompressionMode);

			asset.Metadata = metadata.dump();

			return asset;
		}

		std::vector<char> UnpackTexture(const TextureInfo& info, std::span<const char> src)
		{
			std::vector<char> dest;
			dest.resize(info.BufferSize);
			switch (info.CompressionMode)
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
	}
}