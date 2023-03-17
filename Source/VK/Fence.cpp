#include <PCH.h>
#include <VK/Fence.h>
#include <VK/VulkanRHI.h>

namespace sy
{
	namespace vk
	{
		Fence::Fence(std::string_view name, const VulkanRHI& vulkanRHI, const bool bIsSignaled)
			: VulkanWrapper<VkFence>(
				name, vulkanRHI, VK_OBJECT_TYPE_FENCE, VK_DESTROY_LAMBDA_SIGNATURE(VkFence) {
					vkDestroyFence(vulkanRHI.GetDevice(), handle, nullptr);
				})
		{
			const VkFenceCreateInfo createInfo{
				.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
				.pNext = nullptr,
				.flags = bIsSignaled ? VK_FENCE_CREATE_SIGNALED_BIT : static_cast<VkFlags>(0)
			};

			Native_t handle = VK_NULL_HANDLE;
			VK_ASSERT(vkCreateFence(vulkanRHI.GetDevice(), &createInfo, nullptr, &handle), "Failed to create fence.");
			UpdateHandle(handle);
		}

		void Fence::Wait() const
		{
			const VulkanRHI& vulkanRHI = GetRHI();
			const Native_t handle = GetNativeHandle();
			VK_ASSERT(vkWaitForFences(vulkanRHI.GetDevice(), 1, &handle, VK_TRUE, std::numeric_limits<uint64_t>::max()),
				"Failed to wait fence {}", GetName());
		}

		void Fence::Reset() const
		{
			const VulkanRHI& vulkanRHI = GetRHI();
			const Native_t handle = GetNativeHandle();
			VK_ASSERT(vkResetFences(vulkanRHI.GetDevice(), 1, &handle), "Failed to reset fence {}", GetName());
		}

		bool Fence::IsSignaled() const
		{
			const VulkanRHI& vulkanRHI = GetRHI();
			const Native_t handle = GetNativeHandle();
			const auto status = vkGetFenceStatus(vulkanRHI.GetDevice(), handle);
			return status == VK_SUCCESS;
		}
	} // namespace vk
} // namespace sy
