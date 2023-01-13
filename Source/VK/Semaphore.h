#pragma once
#include <PCH.h>

namespace sy
{
	namespace vk
	{
		class VulkanContext;
		class Semaphore : public VulkanWrapper<VkSemaphore>
		{
		public:
			Semaphore(std::string_view name, const VulkanContext& vulkanContext);
			virtual ~Semaphore() = default;

		};
	}
}
