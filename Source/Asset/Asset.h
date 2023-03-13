#pragma once
#include <PCH.h>
#include <Core/ResourceCache.h>

namespace sy::asset
{
	/**
	 * Asset.Texture is metadata of asset.
	 * Asset.Texture.bin is blob of asset.
	 */
	enum class EAsset
	{
		Texture,
		Model,
		Material,
		Unknown
	};

	inline fs::path PathToBlobPath(const fs::path& path)
	{
		fs::path newPath = path;
		newPath.concat(".blob");
		return newPath;
	}

	template <typename T>
	class AssetData : public NonCopyable
	{
	public:
		AssetData(const fs::path& path) :
			AssetData(path, LoadMetadata(path), LoadBlob(path))
		{
		}

		AssetData(const fs::path& path, const nlohmann::json metadata, std::vector<char> blob) :
			path(path),
			metadata(metadata),
			blob(std::move(blob))
		{
		}

		[[nodiscard]] fs::path GetPath() const
		{
			return path;
		}

		[[nodiscard]] const nlohmann::json& GetMetadata() const
		{
			return metadata;
		}

		[[nodiscard]] const std::vector<char>& GetBlob() const
		{
			return blob;
		}

		void SaveMetadata() const
		{
			const fs::path& path = GetPath();
			std::ofstream outFileStream;
			outFileStream.open(path, std::ios::out | std::ios::trunc);
			if (outFileStream.is_open())
			{
				const std::string jsonStr = metadata.dump(4, ' ');
				outFileStream.write(jsonStr.data(), jsonStr.size());
				outFileStream.close();
			}
		}

		void SaveBlob() const
		{
			const fs::path blobPath = PathToBlobPath(GetPath());
			std::ofstream outFileStream;
			outFileStream.open(blobPath, std::ios::binary | std::ios::out | std::ios::trunc);
			if (outFileStream.is_open())
			{
				const auto& blob = GetBlob();
				outFileStream.write(blob.data(), blob.size());
				outFileStream.close();
			}
		}

	private:
		static nlohmann::json LoadMetadata(const fs::path& path)
		{
			std::ifstream inFileStream;
			inFileStream.open(path);
			if (!inFileStream.is_open())
			{
				spdlog::warn("Failed to load asset file from {}.", path.string());
				return {};
			}

			const std::string jsonStr = InputFileStreamToString(inFileStream);
			return nlohmann::json::parse(jsonStr);
		}

		static std::vector<char> LoadBlob(const fs::path& path)
		{
			const auto blobPath = PathToBlobPath(path);
			std::vector<char> buffer;
			std::ifstream inFileStream;
			inFileStream.open(blobPath, std::ios::binary);

			if (inFileStream.is_open())
			{
				const auto size = QuerySizeOfStream(inFileStream);
				buffer.resize(size);
				inFileStream.read(buffer.data(), size);
			}

			return buffer;
		}

	private:
		fs::path path;
		nlohmann::json metadata;
		std::vector<char> blob;
	};

	template <typename T>
	Handle<AssetData<T>> LoadOrCreateAssetData(const fs::path& path, ResourceCache& resourceCache)
	{
		const auto pathStr = path.string();
		if (!resourceCache.Contains<AssetData<T>>(pathStr))
		{
			const auto newHandle = resourceCache.Add<AssetData<T>>(pathStr);
			resourceCache.SetAlias(pathStr, newHandle);
			return newHandle;
		}

		return resourceCache.QueryAlias<AssetData<T>>(pathStr);
	}
}
