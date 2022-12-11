#pragma once
#include <Core/Core.h>
#include <Asset/AssetCore.h>

namespace sy::vk
{
	class VulkanContext;
	class Texture2D;
	class CommandPoolManager;
	class FrameTracker;
}

namespace sy::asset
{
	enum class ETextureExtension
	{
		PNG,
		JPEG,
		JPG,
		HDR,
		Unknown
	};

	enum class ETextureAssetType
	{
		Texture2D,
		Texture2DArray,
		Texture3D,
		TextureCube,
	};

	VkFormat ExtensionToFormat(ETextureExtension extension);

	struct TextureMetadata
	{
		uint64_t BufferSize = 0;
		//ETextureAssetType Type = ETextureAssetType::Texture2D;
		VkFormat Format = VK_FORMAT_UNDEFINED;
		ECompressionMode CompressionMode = ECompressionMode::LZ4;
		Extent3D<uint32_t> Extent = { 1, 1, 1 };
		std::string SrcPath;
	};

	std::optional<TextureMetadata> ParseMetadata(const Asset& asset);
	std::optional<Asset> Pack(TextureMetadata& metadata, const void* pixelData);
	std::vector<char> Unpack(const TextureMetadata& metadata, std::span<const char> src);

	std::unique_ptr<vk::Texture2D> LoadTextureFromAsset(std::string_view assetPath, const vk::VulkanContext& vulkanContext, const vk::FrameTracker& frameTracker, vk::CommandPoolManager& cmdPoolManager);
}