#include <Core/Core.h>
#include <Asset/AssetConverter.h>
#include <Asset/AssetCore.h>
#include <Asset/TextureAsset.h>
#include <Asset/MeshAsset.h>

namespace sy::asset::convert
{
	bool CovertTexture2D(const fs::path& input, const fs::path& output)
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

		const auto srcExtension = magic_enum::enum_cast<texture::EExtension>(inputExtension);
		if (!srcExtension.has_value())
		{
			SY_ASSERT(false, "Invalid input texture extension {}.", inputExtension)
			return false;
		}

		const auto requiredFormat = texture::ExtensionToFormat(srcExtension.value());
		const auto requiredChannels = vk::ToNumberOfComponents(requiredFormat);

		int width, height, channels;
		stbi_uc* pixels = stbi_load(inputPathStr.c_str(), &width, &height, &channels, requiredChannels);
		if (pixels == nullptr)
		{
			SY_ASSERT(false, "Failed load data from file {}.", input.string());
			return false;
		}

		const auto requiredSize = vk::ToByteSize(requiredFormat) * width * height;

		texture::Metadata metadata;
		metadata.BufferSize = requiredSize;
		metadata.Extent = Extent3D<uint32_t>{ static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1 };
		metadata.Format = requiredFormat;
		metadata.SrcPath = inputPathStr;

		const auto newAssetOpt = texture::Pack(metadata, pixels);
		if (!newAssetOpt.has_value())
		{
			spdlog::warn("Failed to packing texture {}.", inputPathStr);
			return false;
		}

		stbi_image_free(pixels);

		SaveBinary(output.string(), newAssetOpt.value());
		return true;
	}
}
