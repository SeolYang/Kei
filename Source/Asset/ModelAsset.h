#pragma once
#include <PCH.h>
#include <Asset/Asset.h>
#include <Render/Mesh.h>
#include <Component/StaticMeshComponent.h>

/**
 * Mesh Asset structure
 * 1. Asset metadata
 * 2. Mesh metadata
 * 3. Compressed Vertex data
 * 4. Compressed Index data
 */

namespace sy
{
	class ResourceCache;
}

namespace sy::vk
{
	class VulkanContext;
	class CommandPoolManager;
	class FrameTracker;
	class DescriptorManager;
}

namespace sy::render
{
	class Model;
}

namespace sy::asset
{
	std::vector<component::StaticMeshComponent> LoadModel(const std::string& name, const fs::path& path,
	                                                      ResourceCache& resourceCache,
	                                                      const vk::VulkanContext& vulkanContext);
	bool ConvertModel(const fs::path& path);
}
