#pragma once
#include <Core.h>

namespace sy
{
	class VulkanContext;
	class Window;
	class Swapchain : public VulkanWrapper<VkSwapchainKHR>
	{
	public:
		Swapchain(const Window& window, const VulkanContext& vulkanContext);
		virtual ~Swapchain() override;

		void AcquireNext(const Semaphore& presentSemaphore);
		[[nodiscard]] auto GetCurrentImage() const { return images[currentImageIdx]; }
		[[nodiscard]] auto GetCurrentImageView() const { return imageViews[currentImageIdx]; }
		[[nodiscard]] auto GetCurrentImageIndex() const { return currentImageIdx; }

	private:
		const Window& window;
		std::vector<VkImage> images;
		std::vector<VkImageView> imageViews;
		VkFormat format;

		uint32_t currentImageIdx;


	};
}