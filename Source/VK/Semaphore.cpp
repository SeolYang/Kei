#include <Core.h>
#include <VK/Semaphore.h>
#include <Vk/VulkanInstance.h>

namespace sy
{
	Semaphore::Semaphore(const std::string_view name, const VulkanInstance& vulkanInstance) :
		VulkanWrapper<VkSemaphore>(name, vulkanInstance, VK_DESTROY_LAMBDA_SIGNATURE(VkSemaphore)
		{
			vkDestroySemaphore(vulkanInstance.GetLogicalDevice(), handle, nullptr);
		})
	{
		const VkSemaphoreCreateInfo createInfo
		{
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0
		};

		vkCreateSemaphore(vulkanInstance.GetLogicalDevice(), &createInfo, nullptr, &handle);
	}
}
