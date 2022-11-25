#pragma once
#include <Core.h>

namespace sy
{
	class VulkanInstance;
	class Window;
	class Semaphore;
	class Swapchain : public VulkanWrapper<VkSwapchainKHR>
	{
	public:
		Swapchain(const Window& window, const VulkanInstance& vulkanInstance);
		virtual ~Swapchain() override;

		void AcquireNext();
		[[nodiscard]] auto GetCurrentImage() const { return images[currentImageIdx]; }
		[[nodiscard]] auto GetCurrentImageView() const { return imageViews[currentImageIdx]; }
		[[nodiscard]] auto GetCurrentImageIndex() const { return currentImageIdx; }

		Semaphore& GetSemaphore() const { return *presentSemaphore; }

	private:
		const Window& window;
		std::vector<VkImage> images;
		std::vector<VkImageView> imageViews;
		VkFormat format;

		uint32_t currentImageIdx;

		std::unique_ptr<Semaphore> presentSemaphore;

	};
}