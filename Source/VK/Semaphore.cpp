#include <PCH.h>
#include <VK/Semaphore.h>
#include <Vk/VulkanRHI.h>

namespace sy
{
	namespace vk
	{
		Semaphore::Semaphore(const std::string_view name, const VulkanRHI& vulkanRHI)
			: VulkanWrapper<VkSemaphore>(name, vulkanRHI, VK_OBJECT_TYPE_SEMAPHORE)
		{
			const VkSemaphoreCreateInfo createInfo{
				.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
				.pNext = nullptr,
				.flags = 0
			};

			NativeHandle handle = VK_NULL_HANDLE;
			vkCreateSemaphore(vulkanRHI.GetDevice(), &createInfo, nullptr, &handle);

			UpdateHandle(
				handle, SY_VK_WRAPPER_DELETER(rhi) {
					vkDestroySemaphore(rhi.GetDevice(), handle, nullptr);
				});
		}
	} // namespace vk
} // namespace sy
