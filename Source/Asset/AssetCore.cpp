#include <PCH.h>
#include <Asset/AssetCore.h>

namespace sy
{
	namespace asset
	{
		bool SaveBinary(const std::string_view path, const Asset& asset)
		{
			std::ofstream outFileStream;
			outFileStream.open(path, std::ios::binary | std::ios::out | std::ios::trunc);

			outFileStream.write(asset.Identifier, LengthOfArray(asset.Identifier));

			const auto version = asset.Version;
			outFileStream.write(reinterpret_cast<const char*>(&version), sizeof(asset.Version));

			const auto metadataSize = asset.Metadata.size();
			outFileStream.write(reinterpret_cast<const char*>(&metadataSize), sizeof(metadataSize));
			outFileStream.write(reinterpret_cast<const char*>(asset.Metadata.data()), metadataSize);

			const auto blobSize = asset.Blob.size();
			outFileStream.write(reinterpret_cast<const char*>(&blobSize), sizeof(blobSize));
			outFileStream.write(reinterpret_cast<const char*>(asset.Blob.data()), blobSize);

			outFileStream.close();

			return true;
		}

		std::optional<Asset> LoadBinary(const std::string_view path, const uint32_t version, const bool bIgnoreAssetVersionCheck)
		{
			std::ifstream inFileStream;
			inFileStream.open(path, std::ios::binary);
			if (!inFileStream.is_open())
			{
				spdlog::warn("Failed to load asset file from {}.", path);
				return std::nullopt;
			}

			inFileStream.seekg(0);

			Asset asset;
			inFileStream.read(asset.Identifier, LengthOfArray(asset.Identifier));
			inFileStream.read(reinterpret_cast<char*>(&asset.Version), sizeof(asset.Version));
			const bool bAssetVersionMismatched = asset.Version != version;
			if (!bIgnoreAssetVersionCheck && bAssetVersionMismatched)
			{
				SY_ASSERT(false, "Asset version mismatch: loaded asset version is {}, required version is {}", asset.Version, version);
				return std::nullopt;
			}
			else if (bAssetVersionMismatched)
			{
				spdlog::warn("Asset version mismatch: loaded asset version is {}, required version is {}", asset.Version, version);
			}

			auto metadataSize = asset.Metadata.size();
			inFileStream.read(reinterpret_cast<char*>(&metadataSize), sizeof(metadataSize));
			asset.Metadata.resize(metadataSize);
			inFileStream.read(asset.Metadata.data(), metadataSize);

			auto blobSize = asset.Blob.size();
			inFileStream.read(reinterpret_cast<char*>(&blobSize), sizeof(blobSize));
			asset.Blob.resize(blobSize);
			inFileStream.read(asset.Blob.data(), blobSize);

			return asset;
		}

		std::vector<char> CompressLZ4(const std::span<const char> data)
		{
			const auto compressStagingSize = LZ4_compressBound(static_cast<int>(data.size()));
			std::vector<char> comp;
			comp.resize(compressStagingSize);

			const auto compressedSize = LZ4_compress_default(data.data(), comp.data(), static_cast<int>(data.size()), static_cast<int>(comp.size()));
			comp.resize(compressedSize);
			comp.shrink_to_fit();
			return comp;
		}

		std::vector<char> CompressNone(const std::span<const char> data)
		{
			std::vector<char> comp;
			comp.resize(data.size());
			memcpy(comp.data(), data.data(), data.size());
			return comp;
		}

		std::vector<char> CompressData(const ECompressionMode compressionMode, const std::span<const char> data)
		{
			switch (compressionMode)
			{
			case ECompressionMode::LZ4:
				return CompressLZ4(data);
			}

			return CompressNone(data);
		}

		std::vector<char> UncompressLZ4(const std::span<const char> compressedData, const size_t dataSize)
		{
			std::vector<char> uncompressedData;
			uncompressedData.resize(dataSize);
			LZ4_decompress_safe(compressedData.data(), uncompressedData.data(), static_cast<int>(compressedData.size()), static_cast<int>(uncompressedData.size()));
			return uncompressedData;
		}

		std::vector<char> UncompressNone(const std::span<const char> compressedData, const size_t dataSize)
		{
			std::vector<char> uncompressedData;
			uncompressedData.resize(dataSize);
			memcpy(uncompressedData.data(), compressedData.data(), compressedData.size());
			return uncompressedData;
		}

		std::vector<char> Uncompress(const ECompressionMode compressionMode, const size_t dataSize, std::span<const char> compressedData)
		{
			std::vector<char> uncompressedData;
			uncompressedData.resize(dataSize);

			switch (compressionMode)
			{
			case ECompressionMode::LZ4:
				return UncompressLZ4(compressedData, dataSize);
				break;

			default:
				break;
			}

			return UncompressNone(compressedData, dataSize);
		}
	}
}