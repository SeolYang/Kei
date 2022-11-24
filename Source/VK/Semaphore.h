#pragma once
#include <Core.h>

namespace sy
{
	class VulkanInstance;
	class Semaphore : public VulkanWrapper<VkSemaphore>
	{
	public:
		Semaphore(std::string_view name, const VulkanInstance& vulkanInstance);
		virtual ~Semaphore() = default;

	};
}