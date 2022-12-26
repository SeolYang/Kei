#include <Core/Core.h>
#include <Render/RenderPass.h>
#include <VK/VulkanContext.h>

namespace sy::render
{
	RenderPass::RenderPass(std::string_view name, const vk::VulkanContext& vulkanContext,
		vk::DescriptorManager& descriptorManager, const vk::FrameTracker& frameTracker, const vk::Pipeline& pipeline) :
	NamedType(name),
	vulkanContext(vulkanContext),
	descriptorManager(descriptorManager),
	frameTracker(frameTracker),
	pipeline(pipeline)
	{
	}
}
