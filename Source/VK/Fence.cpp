#include <PCH.h>
#include <VK/Fence.h>
#include <VK/VulkanRHI.h>

namespace sy
{
	namespace vk
	{
		Fence::Fence(const std::string_view name, const VulkanRHI& vulkanRHI, const bool bIsSignaled)
			: VulkanWrapper<VkFence>(name, vulkanRHI, VK_OBJECT_TYPE_FENCE)
		{
			const VkFenceCreateInfo createInfo{
				.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
				.pNext = nullptr,
				.flags = bIsSignaled ? VK_FENCE_CREATE_SIGNALED_BIT : static_cast<VkFlags>(0)
			};

			NativeHandle handle = VK_NULL_HANDLE;
			VK_ASSERT(vkCreateFence(vulkanRHI.GetDevice(), &createInfo, nullptr, &handle), "Failed to create fence.");

			UpdateHandle(
				handle,
				SY_VK_WRAPPER_DELETER(rhi) {
					vkDestroyFence(rhi.GetDevice(), handle, nullptr);
				});
		}

		void Fence::Wait() const
		{
			const VulkanRHI& vulkanRHI = GetRHI();
			const NativeHandle handle = GetNative();
			VK_ASSERT(vkWaitForFences(vulkanRHI.GetDevice(), 1, &handle, VK_TRUE, std::numeric_limits<uint64_t>::max()),
				"Failed to wait fence {}", GetName());
		}

		void Fence::Reset() const
		{
			const VulkanRHI& vulkanRHI = GetRHI();
			const NativeHandle handle = GetNative();
			VK_ASSERT(vkResetFences(vulkanRHI.GetDevice(), 1, &handle), "Failed to reset fence {}", GetName());
		}

		bool Fence::IsSignaled() const
		{
			const VulkanRHI& vulkanRHI = GetRHI();
			const auto status = vkGetFenceStatus(vulkanRHI.GetDevice(), GetNative());
			return status == VK_SUCCESS;
		}
	} // namespace vk
} // namespace sy
