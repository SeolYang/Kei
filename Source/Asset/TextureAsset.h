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

namespace sy::asset::texture
{
	enum class EExtension
	{
		PNG,
		JPEG,
		JPG,
		HDR,
		Unknown
	};

	VkFormat ExtensionToFormat(EExtension extension);

	struct Metadata
	{
		uint64_t BufferSize = 0;
		VkFormat Format = VK_FORMAT_UNDEFINED;
		ECompressionMode CompressionMode = ECompressionMode::LZ4;
		Extent3D<uint32_t> Extent = { 1, 1, 1 };
		std::string SrcPath;
	};

	std::optional<Metadata> ParseMetadata(const Asset& asset);
	std::optional<Asset> Pack(Metadata& metadata, const void* pixelData);
	std::vector<char> Unpack(const Metadata& metadata, std::span<const char> src);

	std::unique_ptr<vk::Texture2D> LoadTextureFromAsset(std::string_view assetPath, const vk::VulkanContext& vulkanContext, const vk::FrameTracker& frameTracker, vk::CommandPoolManager& cmdPoolManager);
}