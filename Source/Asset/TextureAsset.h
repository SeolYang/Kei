#pragma once
#include <Core/Core.h>
#include <Asset/AssetCore.h>

/**
 * Texture Asset structure
 * 1. Asset metadata
 * 2. Texture metadata
 * 3. Compressed Texture data
 */

namespace sy::vk
{
	class VulkanContext;
	class Texture2D;
	class CommandPoolManager;
	class FrameTracker;
}

namespace sy::asset
{
	enum class ETextureAssetType
	{
		Texture2D,
		Texture2DArray,
		Texture3D,
		TextureCube,
	};

	struct TextureMetadata
	{
		uint64_t BufferSize = 0;
		/** @todo considering texture type */
		//ETextureAssetType Type = ETextureAssetType::Texture2D;
		VkFormat Format = VK_FORMAT_UNDEFINED;
		ECompressionMode CompressionMode = ECompressionMode::LZ4;
		Extent3D<uint32_t> Extent = { 1, 1, 1 };
		std::string SrcPath;
	};

	inline VkFormat ExtensionToFormat(ETextureExtension extension)
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

	inline bool CheckTextureCompressionSupport(const ECompressionMode compression)
	{
		static robin_hood::unordered_set<ECompressionMode> SupportedCompression
		{
			ECompressionMode::LZ4
		};

		return SupportedCompression.find(compression) != SupportedCompression.end();
	}

	std::optional<TextureMetadata> ParseTextureMetadata(const Asset& asset);
	std::optional<Asset> Pack(const TextureMetadata& metadata, const void* pixelData);
	std::vector<char> Unpack(const TextureMetadata& metadata, std::span<const char> src);

	bool ConvertTexture2D(const fs::path& input);
	bool ConvertTexture2D(const fs::path& input, const fs::path& output);

	std::unique_ptr<vk::Texture> LoadTexture2DFromAsset(std::string_view assetPath, const vk::VulkanContext& vulkanContext, const vk::FrameTracker& frameTracker, vk::CommandPoolManager& cmdPoolManager);

}