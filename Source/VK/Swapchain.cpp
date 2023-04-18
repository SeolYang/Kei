#include <PCH.h>
#include <Window/Window.h>
#include <VK/VulkanContext.h>
#include <VK/VulkanRHI.h>
#include <VK/Swapchain.h>
#include <Vk/Semaphore.h>

namespace sy
{
namespace vk
{
Swapchain::Swapchain(const window::Window& window, VulkanContext& vulkanContext) :
    VulkanWrapper<VkSwapchainKHR>("Swapchain", vulkanContext, VK_OBJECT_TYPE_SWAPCHAIN_KHR),
    window(window),
    currentImageIdx(0)
{
    const auto&           vulkanRHI = vulkanContext.GetRHI();
    vkb::SwapchainBuilder swapchainBuilder{
        vulkanRHI.GetPhysicalDevice(),
        vulkanRHI.GetDevice(),
        vulkanRHI.GetSurface()};

    spdlog::trace("Creating swapchain..");
    const auto     windowExtent = window.GetExtent();
    vkb::Swapchain vkbSwapchain = swapchainBuilder
                                      .use_default_format_selection()
                                      .set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
                                      .set_desired_extent(windowExtent.width, windowExtent.height)
                                      .build()
                                      .value();

    images = vkbSwapchain.get_images().value();
    SY_ASSERT(images.size() > 0, "Images.size() == 0");
    imageViews = vkbSwapchain.get_image_views().value();
    SY_ASSERT(imageViews.size() > 0, "ImageViews.size() == 0");
    format = vkbSwapchain.image_format;
    SY_ASSERT(format != VK_FORMAT_UNDEFINED, "Swapchain format is undefined.");

    const auto handle = vkbSwapchain.swapchain;
    UpdateHandle(
        handle, [handle, imageViewsClone = std::vector{imageViews}](const VulkanRHI& rhi) {
            const auto device = rhi.GetDevice();
            for (const auto imageView : imageViewsClone)
            {
                vkDestroyImageView(device, imageView, nullptr);
            }

            vkDestroySwapchainKHR(device, handle, nullptr);
        });
}

void Swapchain::AcquireNext(const Semaphore& presentSemaphore)
{
    SY_ASSERT(presentSemaphore.IsBinarySemaphore(), "Given semaphore is not type of binary!");
    const auto& vulkanRHI = GetRHI();
    VK_ASSERT(vkAcquireNextImageKHR(
                  vulkanRHI.GetDevice(),
                  this->GetNative(),
                  std::numeric_limits<uint64_t>::max(),
                  presentSemaphore.GetNative(), VK_NULL_HANDLE, &currentImageIdx),
              "Failed to acquire next image view.");
}
} // namespace vk
} // namespace sy
