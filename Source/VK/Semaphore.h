#pragma once
#include <PCH.h>

namespace sy::vk
{
	class VulkanContext;
	class Semaphore : public VulkanWrapper<VkSemaphore>
	{
	public:
		Semaphore(std::string_view name, const VulkanContext& vulkanContext);
		virtual ~Semaphore() = default;

	};
}
