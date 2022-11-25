#include <Core.h>
#include <Window.h>
#include <VK/VulkanInstance.h>
#include <VK/Swapchain.h>
#include <Vk/Semaphore.h>

namespace sy
{
	Swapchain::Swapchain(const Window& window, const VulkanInstance& vulkanInstance) :
		VulkanWrapper("Swapchain", vulkanInstance, VK_DESTROY_LAMBDA_SIGNATURE(VkSwapchainKHR)
		{
			vkDestroySwapchainKHR(vulkanInstance.GetLogicalDevice(), handle, nullptr);
		}),
		window(window),
		currentImageIdx(0),
		presentSemaphore(std::make_unique<Semaphore>("Present Semaphore", vulkanInstance))
	{
		vkb::SwapchainBuilder swapchainBuilder
		{
			vulkanInstance.GetPhysicalDevice(),
			vulkanInstance.GetLogicalDevice(),
			vulkanInstance.GetSurface()
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
		const auto device = vulkanInstance.GetLogicalDevice();
		for (const auto imageView : imageViews)
		{
			vkDestroyImageView(device, imageView, nullptr);
		}
	}

	void Swapchain::AcquireNext()
	{
		VK_ASSERT(vkAcquireNextImageKHR(vulkanInstance.GetLogicalDevice(), handle, std::numeric_limits<uint64_t>::max(), presentSemaphore->GetNativeHandle(), VK_NULL_HANDLE, &currentImageIdx), "Failed to acquire next image view.");
	}
}
