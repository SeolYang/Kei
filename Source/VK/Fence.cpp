#include <PCH.h>
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

			Native_t handle = VK_NULL_HANDLE;
			VK_ASSERT(vkCreateFence(vulkanContext.GetDevice(), &createInfo, nullptr, &handle), "Failed to create fence.");
			UpdateHandle(handle);
		}

		void Fence::Wait() const
		{
			const VulkanContext& vulkanContext = GetContext();
			const Native_t handle = GetNativeHandle();
			VK_ASSERT(vkWaitForFences(vulkanContext.GetDevice(), 1, &handle, VK_TRUE, std::numeric_limits<uint64_t>::max()), "Failed to wait fence {}", GetName());
		}

		void Fence::Reset() const
		{
			const VulkanContext& vulkanContext = GetContext();
			const Native_t handle = GetNativeHandle();
			VK_ASSERT(vkResetFences(vulkanContext.GetDevice(), 1, &handle), "Failed to reset fence {}", GetName());
		}

		bool Fence::IsSignaled() const
		{
			const VulkanContext& vulkanContext = GetContext();
			const Native_t handle = GetNativeHandle();
			const auto status = vkGetFenceStatus(vulkanContext.GetDevice(), handle);
			return status == VK_SUCCESS;
		}

	}
}
