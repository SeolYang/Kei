#pragma once
#include <PCH.h>
#include <Asset/Asset.h>

/**
 * Texture Asset structure
 * 1. Asset metadata
 * 2. Texture metadata
 * 3. Compressed Texture data
 */
namespace sy
{
	class ResourceCache;
}

namespace sy::vk
{
	class Texture;
	class VulkanContext;
} // namespace sy::vk

namespace sy::asset
{
	enum class ETextureExtension
	{
		PNG,
		JPEG,
		JPG,
		HDR
	};

	inline VkFormat ExtensionToFormat(const ETextureExtension extension)
	{
		static const robin_hood::unordered_map<ETextureExtension, VkFormat> Table{
			{ ETextureExtension::HDR, VK_FORMAT_R32G32B32_SFLOAT },
			{ ETextureExtension::PNG, VK_FORMAT_R8G8B8A8_SRGB },
			{ ETextureExtension::JPEG, VK_FORMAT_R8G8B8A8_SRGB },
			{ ETextureExtension::JPG, VK_FORMAT_R8G8B8A8_SRGB }
		};

		return Table.find(extension)->second;
	}

	Handle<vk::Texture> LoadTexture2DFromAsset(const fs::path& path, HandleManager& handleManager,
		vk::VulkanContext& vulkanContext);
	bool ConvertTexture2D(const fs::path& input);
} // namespace sy::asset
