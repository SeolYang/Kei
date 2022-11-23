#include <Core.h>
#include <Fence.h>

#include "VulkanInstance.h"

namespace sy
{
	Fence::Fence(std::string_view name, const VulkanInstance& vulkanInstance, const bool bIsSignaled) :
		NamedType(name),
		vulkanInstance(vulkanInstance)
	{
		const VkFenceCreateInfo createInfo
		{
			.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
			.pNext = nullptr,
			.flags = bIsSignaled ? VK_FENCE_CREATE_SIGNALED_BIT : static_cast<VkFlags>(0)
		};

		VK_ASSERT(vkCreateFence(vulkanInstance.GetLogicalDevice(), &createInfo, nullptr, &fence), "Failed to create fence.");
	}

	Fence::~Fence()
	{
		vkDestroyFence(vulkanInstance.GetLogicalDevice(), fence, nullptr);
	}

	const Fence& Fence::Wait() const
	{
		vkWaitForFences(vulkanInstance.GetLogicalDevice(), 1, &fence, VK_TRUE, std::numeric_limits<uint64_t>::max());
		return *this;
	}

	const Fence& Fence::Reset() const
	{
		vkResetFences(vulkanInstance.GetLogicalDevice(), 1, &fence);
		return *this;
	}

	bool Fence::IsSignaled() const
	{
		return vkGetFenceStatus(vulkanInstance.GetLogicalDevice(), fence) == VK_SUCCESS;
	}

}
