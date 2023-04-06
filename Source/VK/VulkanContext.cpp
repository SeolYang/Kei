#include <PCH.h>
#include <VK/VulkanContext.h>
#include <VK/VulkanRHI.h>
#include <VK/CommandPoolAllocator.h>
#include <VK/DescriptorManager.h>
#include <VK/FrameTracker.h>
#include <VK/LayoutCache.h>
#include <VK/Swapchain.h>

namespace sy::vk
{
VulkanContext::VulkanContext(const window::Window& window) :
    window(window),
    vulkanRHI(std::make_unique<VulkanRHI>(*this, window)),
    frameTracker(std::make_unique<FrameTracker>(*this)),
    cmdPoolAllocator(std::make_unique<CommandPoolAllocator>(*this, *frameTracker)),
    descriptorManager(std::make_unique<DescriptorManager>(*this, *frameTracker)),
    pipelineLayoutCache(std::make_unique<PipelineLayoutCache>(*this))
{
}

VulkanContext::~VulkanContext()
{
}

void VulkanContext::Startup()
{
    spdlog::info("Startup Vulkan Context.");
    vulkanRHI->Startup();
    frameTracker->Startup();
    cmdPoolAllocator->Startup();
    descriptorManager->Startup();
    pipelineLayoutCache->Startup();

    swapchain = std::make_unique<Swapchain>(window, *this);
}

void VulkanContext::Shutdown()
{
    spdlog::info("Shutdown Vulkan Context.");
    vulkanRHI->WaitForDeviceIdle();

    pipelineLayoutCache->Shutdown();
    descriptorManager->Shutdown();
    cmdPoolAllocator->Shutdown();
    frameTracker->Shutdown();
    swapchain.reset();
    FlushDeferredDeallocations();
    vulkanRHI->Shutdown();
}

CommandPoolAllocator& VulkanContext::GetCommandPoolAllocator() const
{
    return *cmdPoolAllocator;
}

DescriptorManager& VulkanContext::GetDescriptorManager() const
{
    return *descriptorManager;
}

VulkanRHI& VulkanContext::GetRHI() const
{
    return *vulkanRHI;
}

FrameTracker& VulkanContext::GetFrameTracker() const
{
    return *frameTracker;
}

PipelineLayoutCache& VulkanContext::GetPipelineLayoutCache() const
{
    return *pipelineLayoutCache;
}

void VulkanContext::BeginFrame()
{
    frameTracker->BeginFrame();
}

void VulkanContext::EndFrame()
{
    frameTracker->EndFrame();
}

void VulkanContext::BeginRender()
{
    frameTracker->WaitForInFlightRenderFence();
    FlushDeferredDeallocations();
    cmdPoolAllocator->BeginFrame();
    descriptorManager->BeginFrame();
}

void VulkanContext::EndRender()
{
    descriptorManager->EndFrame();
    cmdPoolAllocator->EndFrame();
}

void VulkanContext::EnqueueDeferredDeallocation(VulkanObjectDeleter deleter)
{
    this->deferredObjectDeallocations.emplace_back(std::move(deleter));
}

void VulkanContext::FlushDeferredDeallocations()
{
    for (auto& deleter : deferredObjectDeallocations)
    {
        deleter(*vulkanRHI);
    }
    deferredObjectDeallocations.clear();
}

Swapchain& VulkanContext::GetSwapchain() const
{
    return *swapchain;
}

} // namespace sy::vk
