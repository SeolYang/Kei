#include <Core.h>
#include <Window.h>
#include <VK/VulkanContext.h>
#include <VK/Swapchain.h>
#include <Vk/Semaphore.h>

namespace sy
{
	Swapchain::Swapchain(const Window& window, const VulkanContext& vulkanContext) :
		VulkanWrapper<VkSwapchainKHR>("Swapchain", vulkanContext, VK_DESTROY_LAMBDA_SIGNATURE(VkSwapchainKHR)
		{
			vkDestroySwapchainKHR(vulkanContext.GetDevice(), handle, nullptr);
		}),
		window(window),
		currentImageIdx(0)
	{
		vkb::SwapchainBuilder swapchainBuilder
		{
			vulkanContext.GetPhysicalDevice(),
			vulkanContext.GetDevice(),
			vulkanContext.GetSurface()
		};

		spdlog::trace("Creating swapchain..");
		const auto windowExtent = window.GetExtent();
		vkb::Swapchain vkbSwapchain = swapchainBuilder
			.use_default_format_selection()
			.set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
			.set_desired_extent(windowExtent.width, windowExtent.height)
			.build()
			.value();

		handle = vkbSwapchain.swapchain;
		SY_ASSERT(handle != VK_NULL_HANDLE, "Invalid swapchain.");
		images = vkbSwapchain.get_images().value();
		imageViews = vkbSwapchain.get_image_views().value();
		format = vkbSwapchain.image_format;
	}

	Swapchain::~Swapchain()
	{
		const auto device = vulkanContext.GetDevice();
		for (const auto imageView : imageViews)
		{
			vkDestroyImageView(device, imageView, nullptr);
		}
	}

	void Swapchain::AcquireNext(const Semaphore& presentSemaphore)
	{
		VK_ASSERT(vkAcquireNextImageKHR(vulkanContext.GetDevice(), handle, std::numeric_limits<uint64_t>::max(), presentSemaphore.GetNativeHandle(), VK_NULL_HANDLE, &currentImageIdx), "Failed to acquire next image view.");
	}
}
