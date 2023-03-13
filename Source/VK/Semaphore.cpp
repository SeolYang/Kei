#include <PCH.h>
#include <VK/Semaphore.h>
#include <Vk/VulkanRHI.h>

namespace sy
{
	namespace vk
	{
		Semaphore::Semaphore(const std::string_view name, const VulkanRHI& vulkanRHI) :
			VulkanWrapper<VkSemaphore>(name, vulkanRHI, VK_OBJECT_TYPE_SEMAPHORE,
			                           VK_DESTROY_LAMBDA_SIGNATURE(VkSemaphore)
			                           {
				                           vkDestroySemaphore(vulkanRHI.GetDevice(), handle, nullptr);
			                           })
		{
			const VkSemaphoreCreateInfo createInfo
			{
				.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
				.pNext = nullptr,
				.flags = 0
			};

			Native_t handle = VK_NULL_HANDLE;
			vkCreateSemaphore(vulkanRHI.GetDevice(), &createInfo, nullptr, &handle);
			UpdateHandle(handle);
		}
	}
}
