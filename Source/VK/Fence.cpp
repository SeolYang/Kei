#include <Core.h>
#include <VK/Fence.h>
#include <VK/VulkanInstance.h>

namespace sy
{
	Fence::Fence(std::string_view name, const VulkanInstance& vulkanInstance, const bool bIsSignaled) :
		VulkanWrapper<VkFence>(name, vulkanInstance, VK_DESTROY_LAMBDA_SIGNATURE(VkFence)
		{
			vkDestroyFence(vulkanInstance.GetLogicalDevice(), handle, nullptr);
		})
	{
		const VkFenceCreateInfo createInfo
		{
			.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
			.pNext = nullptr,
			.flags = bIsSignaled ? VK_FENCE_CREATE_SIGNALED_BIT : static_cast<VkFlags>(0)
		};

		VK_ASSERT(vkCreateFence(vulkanInstance.GetLogicalDevice(), &createInfo, nullptr, &handle), "Failed to create fence.");
	}

	void Fence::Wait() const
	{
		vkWaitForFences(vulkanInstance.GetLogicalDevice(), 1, &handle, VK_TRUE, std::numeric_limits<uint64_t>::max());
	}

	void Fence::Reset() const
	{
		vkResetFences(vulkanInstance.GetLogicalDevice(), 1, &handle);
	}

	bool Fence::IsSignaled() const
	{
		return vkGetFenceStatus(vulkanInstance.GetLogicalDevice(), handle) == VK_SUCCESS;
	}

}
