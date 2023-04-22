#include <PCH.h>
#include <VK/Semaphore.h>
#include <VK/VulkanContext.h>
#include <Vk/VulkanRHI.h>

namespace sy::vk
{
Semaphore::Semaphore(const std::string_view name, VulkanContext& vulkanContext,  const bool bIsBinarySemaphore) :
    VulkanWrapper<VkSemaphore>(name, vulkanContext, VK_OBJECT_TYPE_SEMAPHORE),
    bIsBinarySemaphore(bIsBinarySemaphore)
{
    const VkSemaphoreTypeCreateInfo typeCreateInfo{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO,
        .pNext = nullptr,
        .semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE,
        .initialValue = 0};

    const VkSemaphoreCreateInfo createInfo{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        .pNext = bIsBinarySemaphore ? nullptr : &typeCreateInfo,
        .flags = 0};

    NativeHandle handle = VK_NULL_HANDLE;
    const auto& vulkanRHI = vulkanContext.GetRHI();
    vkCreateSemaphore(vulkanRHI.GetDevice(), &createInfo, nullptr, &handle);

    UpdateHandle(
        handle, [handle](const VulkanRHI& rhi) {
            vkDestroySemaphore(rhi.GetDevice(), handle, nullptr);
        });
}

void Semaphore::Wait(const uint64_t timeout) const
{
    SY_ASSERT(!bIsBinarySemaphore, "Semaphore type is not a timeline semaphore.");
    const auto nativeHandle = GetNative();
    const VulkanRHI& rhi = GetRHI();
    const VkSemaphoreWaitInfo waitInfo{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO,
        .pNext = nullptr,
        .semaphoreCount = 1,
        .pSemaphores = &nativeHandle,
        .pValues = &value,
    };

    vkWaitSemaphores(rhi.GetDevice(), &waitInfo, timeout);
}

void Semaphore::Signal() const
{
    const VulkanRHI& rhi = GetRHI();
    const VkSemaphoreSignalInfo signalInfo{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SIGNAL_INFO,
        .pNext = nullptr,
        .semaphore = GetNative(),
        .value = value};

    vkSignalSemaphore(rhi.GetDevice(), &signalInfo);
}
} // namespace sy::vk
