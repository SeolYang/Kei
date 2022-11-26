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
		VK_ASSERT(vkWaitForFences(vulkanInstance.GetLogicalDevice(), 1, &handle, VK_TRUE, std::numeric_limits<uint64_t>::max()), "Failed to wait fence {}", GetName());
	}

	void Fence::Reset() const
	{
		VK_ASSERT(vkResetFences(vulkanInstance.GetLogicalDevice(), 1, &handle), "Failed to reset fence {}", GetName());
	}

	bool Fence::IsSignaled() const
	{
		const auto status = vkGetFenceStatus(vulkanInstance.GetLogicalDevice(), handle);
		return status == VK_SUCCESS;
	}

}
