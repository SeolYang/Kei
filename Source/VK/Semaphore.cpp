#include <PCH.h>
#include <VK/Semaphore.h>
#include <Vk/VulkanContext.h>

namespace sy
{
	namespace vk
	{
		Semaphore::Semaphore(const std::string_view name, const VulkanContext& vulkanContext) :
			VulkanWrapper<VkSemaphore>(name, vulkanContext, VK_OBJECT_TYPE_SEMAPHORE, VK_DESTROY_LAMBDA_SIGNATURE(VkSemaphore)
		{
			vkDestroySemaphore(vulkanContext.GetDevice(), handle, nullptr);
		})
		{
			const VkSemaphoreCreateInfo createInfo
			{
				.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
				.pNext = nullptr,
				.flags = 0
			};

			Native_t handle = VK_NULL_HANDLE;
			vkCreateSemaphore(vulkanContext.GetDevice(), &createInfo, nullptr, &handle);
			UpdateHandle(handle);
		}
	}
}
