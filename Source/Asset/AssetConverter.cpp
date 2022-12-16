#include <Core/Core.h>
#include <Asset/AssetConverter.h>
#include <Asset/AssetCore.h>
#include <Asset/TextureAsset.h>
#include <Asset/MeshAsset.h>

namespace sy::asset
{
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
				switch (const auto assetType = FileExtensionToAssetType(extension); assetType)
				{
				case EAssetType::Texture:
					/** @todo more generic convert methods for texture */
					spdlog::info("Converting texture resource {}.", filePath.string());
					ConvertTexture2D(filePath);
					break;

				case EAssetType::Mesh:
					spdlog::info("Converting mesh resource {}.", filePath.string());
					ConvertMesh(filePath);
					break;
				}
			}
		}
	}
}
