#pragma once
#include <Core.h>

namespace sy
{
	class VulkanContext;
	class Buffer : public VulkanWrapper<VkBuffer>
	{
	public:

	private:
		VmaAllocation allocation;

	};
}