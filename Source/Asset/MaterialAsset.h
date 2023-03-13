#pragma once
#include <PCH.h>
#include <Asset/Asset.h>

namespace sy::vk
{
	class VulkanContext;
	class Texture;
}

namespace sy::render
{
	class Material;
}

namespace sy::asset
{
	Handle<render::Material> LoadMaterialFromAsset(const fs::path& path, ResourceCache& resourceCache,
	                                               const vk::VulkanContext& vulkanContext);
	void CreateMaterial(const fs::path& path);
}
