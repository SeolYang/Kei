#pragma once
#include <PCH.h>
#include <VK/VulkanWrapper.h>

namespace sy::vk
{
	class Semaphore : public VulkanWrapper<VkSemaphore>
	{
	public:
		Semaphore(std::string_view name, VulkanContext& vulkanContext);
		~Semaphore() = default;
	};
} // namespace sy::vk
