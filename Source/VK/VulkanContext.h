#pragma once
#include <PCH.h>

namespace sy::window
{
class Window;
}

namespace sy::vk
{
class VulkanRHI;
class CommandPoolAllocator;
class DescriptorManager;
class FrameTracker;
class PipelineLayoutCache;
class Swapchain;
class VulkanContext : public Subsystem
{
public:
    VulkanContext(const window::Window& window);
    ~VulkanContext();

    void Startup() override;
    void Shutdown() override;

    [[nodiscard]] VulkanRHI& GetRHI() const;
    [[nodiscard]] FrameTracker& GetFrameTracker() const;
    [[nodiscard]] CommandPoolAllocator& GetCommandPoolAllocator() const;
    [[nodiscard]] DescriptorManager& GetDescriptorManager() const;
    [[nodiscard]] PipelineLayoutCache& GetPipelineLayoutCache() const;
    [[nodiscard]] Swapchain& GetSwapchain() const;

    void BeginFrame();
    void EndFrame();

    void BeginRender();
    void EndRender();

    void EnqueueDeferredDeallocation(VulkanObjectDeleter deleter);

private:
    void FlushDeferredDeallocations();

private:
    const window::Window& window;
    std::unique_ptr<VulkanRHI> vulkanRHI;
    std::unique_ptr<FrameTracker> frameTracker;
    std::unique_ptr<CommandPoolAllocator> cmdPoolAllocator;
    std::unique_ptr<DescriptorManager> descriptorManager;
    std::unique_ptr<PipelineLayoutCache> pipelineLayoutCache;
    std::vector<VulkanObjectDeleter> deferredObjectDeallocations;

    std::unique_ptr<Swapchain> swapchain;
};
} // namespace sy::vk
