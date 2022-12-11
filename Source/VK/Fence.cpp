#include <Core/Core.h>
#include <VK/Fence.h>
#include <VK/VulkanContext.h>

namespace sy
{
	namespace vk
	{
		Fence::Fence(std::string_view name, const VulkanContext& vulkanContext, const bool bIsSignaled) :
			VulkanWrapper<VkFence>(name, vulkanContext, VK_OBJECT_TYPE_FENCE, VK_DESTROY_LAMBDA_SIGNATURE(VkFence)
		{
			vkDestroyFence(vulkanContext.GetDevice(), handle, nullptr);
		})
		{
			const VkFenceCreateInfo createInfo
			{
				.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
				.pNext = nullptr,
				.flags = bIsSignaled ? VK_FENCE_CREATE_SIGNALED_BIT : static_cast<VkFlags>(0)
			};

			VK_ASSERT(vkCreateFence(vulkanContext.GetDevice(), &createInfo, nullptr, &handle), "Failed to create fence.");
		}

		void Fence::Wait() const
		{
			VK_ASSERT(vkWaitForFences(vulkanContext.GetDevice(), 1, &handle, VK_TRUE, std::numeric_limits<uint64_t>::max()), "Failed to wait fence {}", GetName());
		}

		void Fence::Reset() const
		{
			VK_ASSERT(vkResetFences(vulkanContext.GetDevice(), 1, &handle), "Failed to reset fence {}", GetName());
		}

		bool Fence::IsSignaled() const
		{
			const auto status = vkGetFenceStatus(vulkanContext.GetDevice(), handle);
			return status == VK_SUCCESS;
		}

	}
}
