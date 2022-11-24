#pragma once
#include <Core.h>

namespace sy
{
	class VulkanInstance;
	class Window;
	class Semaphore;
	class Swapchain
	{
	public:
		Swapchain(const Window& window, const VulkanInstance& vulkanInstance);
		~Swapchain();

		void AcquireNext();
		[[nodiscard]] VkImage GetCurrentImage() const { return images[currentImageIdx]; }
		[[nodiscard]] VkImageView GetCurrentImageView() const { return imageViews[currentImageIdx]; }

		Semaphore& GetSemaphore() const { return *presentSemaphore; }
	private:
		const Window& window;
		const VulkanInstance& vulkanInstance;
		VkSwapchainKHR swapchain;
		std::vector<VkImage> images;
		std::vector<VkImageView> imageViews;
		VkFormat format;

		uint32_t currentImageIdx;

		std::unique_ptr<Semaphore> presentSemaphore;

	};
}