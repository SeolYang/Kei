#pragma once
#include <PCH.h>
#include <VK/VulkanWrapper.h>

namespace sy::window
{
	class Window;
}

namespace sy::vk
{
	class Semaphore;
	class Swapchain : public VulkanWrapper<VkSwapchainKHR>
	{
	public:
		Swapchain(const window::Window& window, VulkanContext& vulkanContext);
		~Swapchain() override = default;

		void AcquireNext(const Semaphore& presentSemaphore);

		[[nodiscard]] auto GetCurrentImage() const
		{
			return images[currentImageIdx];
		}

		[[nodiscard]] auto GetCurrentImageView() const
		{
			return imageViews[currentImageIdx];
		}

		[[nodiscard]] auto GetCurrentImageIndex() const
		{
			return currentImageIdx;
		}

		[[nodiscard]] auto GetColorAttachmentInfo(VkClearColorValue clearColorValue = { 0.f, 0.f, 0.f, 1.f }) const
		{
			return VkRenderingAttachmentInfoKHR{
				.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR,
				.pNext = nullptr,
				.imageView = GetCurrentImageView(),
				.imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
				.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
				.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
				.clearValue = { clearColorValue }
			};
		}

	private:
		const window::Window& window;
		std::vector<VkImage> images = {};
		std::vector<VkImageView> imageViews = {};
		VkFormat format = VK_FORMAT_UNDEFINED;

		uint32_t currentImageIdx = 0;
	};
} // namespace sy::vk
