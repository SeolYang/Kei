#include <PCH.h>
#include <VK/Semaphore.h>
#include <VK/VulkanContext.h>
#include <Vk/VulkanRHI.h>

namespace sy
{
	namespace vk
	{
		Semaphore::Semaphore(const std::string_view name, VulkanContext& vulkanContext)
			: VulkanWrapper<VkSemaphore>(name, vulkanContext, VK_OBJECT_TYPE_SEMAPHORE)
		{
			const VkSemaphoreCreateInfo createInfo{
				.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
				.pNext = nullptr,
				.flags = 0
			};

			NativeHandle handle = VK_NULL_HANDLE;
			const auto& vulkanRHI = vulkanContext.GetRHI();
			vkCreateSemaphore(vulkanRHI.GetDevice(), &createInfo, nullptr, &handle);

			UpdateHandle(
				handle, [handle](const VulkanRHI& rhi) {
					vkDestroySemaphore(rhi.GetDevice(), handle, nullptr);
				});
		}
	} // namespace vk
} // namespace sy
