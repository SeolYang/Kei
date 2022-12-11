#pragma once
#include <Core/Core.h>
#include <Assets/AssetCore.h>

namespace sy
{
	namespace asset
	{
		enum class ETextureFormat : uint32_t
		{
			Unknown = 0,
			RGBA8,
			HDR,
		};

		enum class ETextureExtension
		{
			PNG,
			JPEG,
			JPG,
			HDR,
			Unknown
		};

		constexpr ETextureFormat TextureFormatToExtension(const ETextureExtension extension)
		{
			switch (extension)
			{
			case ETextureExtension::HDR:
				return ETextureFormat::HDR;

			case ETextureExtension::PNG:
			case ETextureExtension::JPEG:
			case ETextureExtension::JPG:
				return ETextureFormat::RGBA8;

			default:
				return ETextureFormat::Unknown;
			}
		}

		struct TextureInfo
		{
			uint64_t BufferSize = 0;
			ETextureFormat Format = ETextureFormat::Unknown;
			ECompressionMode CompressionMode = ECompressionMode::LZ4;
			Extent3D<uint32_t> Extent = { 1, 1, 1 };
			std::string SrcPath;
		};

		namespace texture
		{
			std::optional<TextureInfo> ParseTextureAssetInfo(Asset& asset);
			std::optional<Asset> PackTexture(TextureInfo& info, void* pixelData);
			std::vector<char> UnpackTexture(TextureInfo& info, std::span<const char> src);
		}
		bool CovertTexture(const fs::path& input, const fs::path& output);

	}
}