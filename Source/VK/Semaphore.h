#pragma once
#include <PCH.h>

namespace sy::vk
{
	class VulkanRHI;

	class Semaphore : public VulkanWrapper<VkSemaphore>
	{
	public:
		Semaphore(std::string_view name, const VulkanRHI& vulkanRHI);
		~Semaphore() = default;
	};
} // namespace sy::vk
