#include <PCH.h>
#include <Asset/AssetConverter.h>
#include <Asset/Asset.h>
#include <Asset/TextureAsset.h>
#include <Asset/ModelAsset.h>

namespace sy::asset
{
	std::optional<EAsset> FileExtensionToAssetType(const std::string& extension)
	{
		static const robin_hood::unordered_map<std::string, EAsset> extAssetMap
		{
			{"PNG", EAsset::Texture},
			{"JPG", EAsset::Texture},
			{"JPEG", EAsset::Texture},
			{"GLTF", EAsset::Model},
			{"OBJ", EAsset::Model},
			{"FBX", EAsset::Model},
		};

		const auto found = extAssetMap.find(extension);
		return found != extAssetMap.end() ? std::optional<EAsset>{found->second} : std::nullopt;
	}

	void ConvertAssets(const fs::path& root)
	{
		spdlog::info("Converting assets in {}.", root.string());
		const fs::recursive_directory_iterator itr{ root };
		for (const auto& entry : itr)
		{
			if (entry.is_regular_file())
			{
				const fs::path filePath = fs::relative(entry.path(), root.parent_path());
				std::string extension = filePath.extension().string();
				if (extension.find('.') == 0)
				{
					extension = extension.substr(1);
				}

				std::transform(extension.begin(), extension.end(), extension.begin(), ::toupper);
				switch (const auto assetType = FileExtensionToAssetType(extension).value_or(EAsset::Unknown); assetType)
				{
					case EAsset::Texture:
						/** @todo more generic convert methods for texture */
						spdlog::info("Converting texture resource {}.", filePath.string());
						ConvertTexture2D(filePath);
					break;

					case EAsset::Model:
						spdlog::info("Converting mesh resource {}.", filePath.string());
						ConvertModel(filePath);
					break;

					default:
					break;
				}
			}
		}
	}
}
