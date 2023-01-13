#pragma once
#include <PCH.h>

namespace sy
{
	class Window;
	namespace vk
	{
		class VulkanContext;
		class Semaphore;
		class Swapchain : public VulkanWrapper<VkSwapchainKHR>
		{
		public:
			Swapchain(const Window& window, const VulkanContext& vulkanContext);
			virtual ~Swapchain() override;

			void AcquireNext(const Semaphore& presentSemaphore);
			[[nodiscard]] auto GetCurrentImage() const { return images[currentImageIdx]; }
			[[nodiscard]] auto GetCurrentImageView() const { return imageViews[currentImageIdx]; }
			[[nodiscard]] auto GetCurrentImageIndex() const { return currentImageIdx; }
			[[nodiscard]] auto GetColorAttachmentInfo(VkClearColorValue clearColorValue = { 0.f, 0.f, 0.f, 1.f }) const
			{
				return VkRenderingAttachmentInfoKHR
				{
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
			const Window& window;
			std::vector<VkImage> images;
			std::vector<VkImageView> imageViews;
			VkFormat format;

			uint32_t currentImageIdx;

		};
	}
}
