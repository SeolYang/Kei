#pragma once
#include <PCH.h>
#include <Asset/Asset.h>

namespace sy::vk
{
	class VulkanContext;
	class Texture;
	class CommandPoolManager;
	class DescriptorManager;
	class FrameTracker;
}

namespace sy::render
{
	class Material;
}

namespace sy::asset
{
	Handle<render::Material> LoadMaterialFromAsset(const fs::path& path, ResourceCache& resourceCache, const vk::VulkanContext& vulkanContext, const vk::FrameTracker& frameTracker, vk::CommandPoolManager& cmdPoolManager, vk::DescriptorManager& descriptorManager);
	void CreateMaterial(const fs::path& path);
}