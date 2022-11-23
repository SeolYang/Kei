#include <Core.h>
#include <Window.h>
#include <VK/Swapchain.h>
#include <VK/VulkanInstance.h>

namespace sy
{
	Swapchain::Swapchain(const VulkanInstance& vulkanInstance, const Window& window) :
		vulkanInstance(vulkanInstance),
		window(window),
		swapchain(VK_NULL_HANDLE)
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

        swapchain = vkbSwapchain.swapchain;
        SY_ASSERT(swapchain != VK_NULL_HANDLE, "Invalid swapchain.");
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

        vkDestroySwapchainKHR(device, swapchain, nullptr);
	}
}