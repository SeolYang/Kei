#pragma once
#include <Core.h>

namespace sy
{
	class VulkanContext;
	class Semaphore : public VulkanWrapper<VkSemaphore>
	{
	public:
		Semaphore(std::string_view name, const VulkanContext& vulkanContext);
		virtual ~Semaphore() = default;

	};
}