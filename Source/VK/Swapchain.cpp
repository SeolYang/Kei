#include <PCH.h>
#include <Window/Window.h>
#include <VK/VulkanRHI.h>
#include <VK/Swapchain.h>
#include <Vk/Semaphore.h>

namespace sy
{
	namespace vk
	{
		Swapchain::Swapchain(const window::Window& window, const VulkanRHI& vulkanRHI)
			: VulkanWrapper<VkSwapchainKHR>(
				"Swapchain", vulkanRHI, VK_OBJECT_TYPE_SWAPCHAIN_KHR,
				VK_DESTROY_LAMBDA_SIGNATURE(VkSwapchainKHR) {
					vkDestroySwapchainKHR(vulkanRHI.GetDevice(), handle, nullptr);
				})
			, window(window)
			, currentImageIdx(0)
		{
			vkb::SwapchainBuilder swapchainBuilder{
				vulkanRHI.GetPhysicalDevice(),
				vulkanRHI.GetDevice(),
				vulkanRHI.GetSurface()
			};

			spdlog::trace("Creating swapchain..");
			const auto windowExtent = window.GetExtent();
			vkb::Swapchain vkbSwapchain = swapchainBuilder
											  .use_default_format_selection()
											  .set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
											  .set_desired_extent(windowExtent.width, windowExtent.height)
											  .build()
											  .value();

			UpdateHandle(vkbSwapchain.swapchain);
			images = vkbSwapchain.get_images().value();
			imageViews = vkbSwapchain.get_image_views().value();
			format = vkbSwapchain.image_format;
		}

		Swapchain::~Swapchain()
		{
			const auto& vulkanRHI = GetRHI();
			const auto device = vulkanRHI.GetDevice();
			for (const auto imageView : imageViews)
			{
				vkDestroyImageView(device, imageView, nullptr);
			}
		}

		void Swapchain::AcquireNext(const Semaphore& presentSemaphore)
		{
			const auto& vulkanRHI = GetRHI();
			const auto handle = GetNativeHandle();
			VK_ASSERT(vkAcquireNextImageKHR(vulkanRHI.GetDevice(), handle, std::numeric_limits<uint64_t>::max(),
						  presentSemaphore.GetNativeHandle(), VK_NULL_HANDLE, &currentImageIdx),
				"Failed to acquire next image view.");
		}
	} // namespace vk
} // namespace sy
