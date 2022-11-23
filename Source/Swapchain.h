#pragma once
#include <Core.h>

namespace sy
{
	class VulkanInstance;
	class Window;
	class Swapchain
	{
	public:
		Swapchain(const VulkanInstance& vulkanInstance, const Window& window);
		~Swapchain();

	private:
		const VulkanInstance& vulkanInstance;
		const Window& window;
		VkSwapchainKHR swapchain;
		std::vector<VkImage> images;
		std::vector<VkImageView> imageViews;
		VkFormat format;

	};
}