#include <Core/Core.h>
#include <Asset/AssetCore.h>

namespace sy
{
	namespace asset
	{
		bool SaveBinary(const std::string_view path, const Asset& asset)
		{
			std::ofstream outFileStream;
			outFileStream.open(path, std::ios::binary | std::ios::out);

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

		std::optional<Asset> LoadBinary(const std::string_view path)
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
	}
}