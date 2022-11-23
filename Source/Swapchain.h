#pragma once
#include <Core.h>

namespace sy
{
	class VulkanInstance;
	class Swapchain
	{
	public:
		Swapchain(const VulkanInstance& vulkanInstance);
		~Swapchain();

	private:
		const VulkanInstance& vulkanInstance;

	};
}