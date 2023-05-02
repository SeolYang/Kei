#include <PCH.h>
#include <Window/Window.h>
#include <VK/VulkanRHI.h>
#include <VK/Swapchain.h>
#include <VK/CommandBuffer.h>
#include <VK/CommandPool.h>
#include <VK/Semaphore.h>
#include <VK/Fence.h>
#include <VK/Buffer.h>
#include <VK/Texture.h>
#include <VK/FrameTracker.h>

namespace sy::vk
{
VulkanRHI::VulkanRHI(VulkanContext& vulkanContext, const window::Window& window) :
    vulkanContext(vulkanContext),
    window(window),
    instance(VK_NULL_HANDLE),
    surface(VK_NULL_HANDLE),
    physicalDevice(VK_NULL_HANDLE),
    device(VK_NULL_HANDLE),
    allocator(VK_NULL_HANDLE)
{
}

VulkanRHI::~VulkanRHI()
{
}

void VulkanRHI::Startup()
{
    spdlog::info("Startup Vulkan RHI.");
    volkInitialize();

    vkb::InstanceBuilder instanceBuilder;
    auto instanceBuilderRes = instanceBuilder.set_app_name(window.GetTitle().data())
#ifdef _DEBUG
                                  .request_validation_layers()
                                  .use_default_debug_messenger()
#endif
                                  .require_api_version(1, 3, 0)
                                  .build();

    const auto vkbInstance = instanceBuilderRes.value();
    instance = vkbInstance.instance;
    debugMessenger = vkbInstance.debug_messenger;

    volkLoadInstance(instance);

    SDL_Vulkan_CreateSurface(&window.GetSDLWindow(), instance, &surface);

    vkb::PhysicalDeviceSelector physicalDeviceSelector{vkbInstance};
    auto vkbPhysicalDevice = physicalDeviceSelector.set_minimum_version(1, 3)
                                 .set_surface(surface)
                                 .add_required_extension(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME)
                                 .add_required_extension(VK_KHR_SWAPCHAIN_EXTENSION_NAME)
                                 .add_required_extension(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME)
                                 .add_required_extension(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME)
                                 .add_required_extension(VK_KHR_MAINTENANCE1_EXTENSION_NAME)
                                 .select()
                                 .value();

    physicalDevice = vkbPhysicalDevice.physical_device;
    gpuProperties = vkbPhysicalDevice.properties;
    gpuName = gpuProperties.deviceName;
    spdlog::trace("\n----------- GPU Properties -----------\n* Device Name: {}\n* GPU Vendor ID: {}\n* API Version: {}\n* Driver Version: {}\n* Device ID: {}\n* Max Bound Descriptor Sets: {}\n* Min Uniform Buffer Offset Alignment: {}\n* Min Storage Buffer Offset Alignment: {}\n* Max Frame Buffer Extent: {}x{}\n* Max Memory Allocation Count: {}\n* Max Sampler Allocation Count: {}\n",
                  gpuName,
                  gpuProperties.vendorID,
                  gpuProperties.apiVersion,
                  gpuProperties.driverVersion,
                  gpuProperties.deviceID,
                  gpuProperties.limits.maxBoundDescriptorSets,
                  gpuProperties.limits.minUniformBufferOffsetAlignment,
                  gpuProperties.limits.minStorageBufferOffsetAlignment,
                  gpuProperties.limits.maxFramebufferWidth,
                  gpuProperties.limits.maxFramebufferHeight,
                  gpuProperties.limits.maxMemoryAllocationCount,
                  gpuProperties.limits.maxSamplerAllocationCount);

    vkb::DeviceBuilder deviceBuilder{vkbPhysicalDevice};
    VkPhysicalDeviceDynamicRenderingFeaturesKHR dynamicRenderingFeatures{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES_KHR,
        .pNext = nullptr,
        .dynamicRendering = VK_TRUE};

    VkPhysicalDeviceDescriptorIndexingFeatures descriptorIndexingFeatures{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES,
        .pNext = nullptr,
        .descriptorBindingUniformBufferUpdateAfterBind = VK_TRUE,
        .descriptorBindingSampledImageUpdateAfterBind = VK_TRUE,
        .descriptorBindingStorageImageUpdateAfterBind = VK_TRUE,
        .descriptorBindingStorageBufferUpdateAfterBind = VK_TRUE,
        .descriptorBindingPartiallyBound = VK_TRUE,
        .descriptorBindingVariableDescriptorCount = VK_TRUE,
        .runtimeDescriptorArray = VK_TRUE,
    };

    VkPhysicalDeviceSynchronization2Features synchronization2Features{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES,
        .pNext = nullptr,
        .synchronization2 = true};

	VkPhysicalDeviceTimelineSemaphoreFeatures timelineSemaphoreFeatures{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_FEATURES,
        .pNext = nullptr,
        .timelineSemaphore = true};

    auto vkbDeviceRes =
        deviceBuilder.add_pNext(&dynamicRenderingFeatures)
            .add_pNext(&descriptorIndexingFeatures)
            .add_pNext(&synchronization2Features)
            .add_pNext(&timelineSemaphoreFeatures)
            .build();
    SY_ASSERT(vkbDeviceRes.has_value(), "Failed to create device using GPU {}.", gpuName);
    auto& vkbDevice = vkbDeviceRes.value();
    device = vkbDevice.device;
    spdlog::trace("Succeed to create logical device using GPU {}.", gpuName);

    const VmaVulkanFunctions vkFunctions{
        .vkGetInstanceProcAddr = vkGetInstanceProcAddr,
        .vkGetDeviceProcAddr = vkGetDeviceProcAddr};

    const VmaAllocatorCreateInfo allocatorInfo{
        .physicalDevice = physicalDevice,
        .device = device,
        .pVulkanFunctions = &vkFunctions,
        .instance = instance,
    };
    VK_ASSERT(vmaCreateAllocator(&allocatorInfo, &allocator),
              "Failed to create vulkan memory allocator instance.");

    InitQueues(vkbDevice);
}

void VulkanRHI::Shutdown()
{
    spdlog::info("Shutdown Vulkan RHI.");
    WaitAllQueuesForIdle();
    {
        vmaDestroyAllocator(allocator);
        allocator = VK_NULL_HANDLE;
        vkDestroyDevice(device, nullptr);
        device = VK_NULL_HANDLE;
        vkDestroySurfaceKHR(instance, surface, nullptr);
        surface = VK_NULL_HANDLE;
        vkb::destroy_debug_utils_messenger(instance, debugMessenger, nullptr);
        debugMessenger = VK_NULL_HANDLE;
        vkDestroyInstance(instance, nullptr);
        instance = VK_NULL_HANDLE;
    }
}

uint32_t VulkanRHI::GetQueueFamilyIndex(const EQueueType queueType) const
{
    switch (queueType)
    {
        case EQueueType::Graphics:
            return graphicsQueueFamilyIdx;
        case EQueueType::Compute:
            return computeQueueFamilyIdx;
        case EQueueType::Transfer:
            return transferQueueFamilyIdx;
        case EQueueType::Present:
            return presentQueueFamilyIdx;
    }

    return graphicsQueueFamilyIdx;
}

VkQueue VulkanRHI::GetQueue(const EQueueType queueType) const
{
    switch (queueType)
    {
        case EQueueType::Graphics:
            return graphicsQueue;
        case EQueueType::Compute:
            return computeQueue;
        case EQueueType::Transfer:
            return transferQueue;
        case EQueueType::Present:
            return presentQueue;
    }

    return graphicsQueue;
}

void VulkanRHI::SubmitSync(const EQueueType queueType, const CRefSpan<CommandBuffer> cmdBuffers, const CRefSpan<Semaphore> waitSemaphores, const VkPipelineStageFlags2 waitAt, const RefSpan<Semaphore> signalSemaphores, const VkPipelineStageFlags2 signalAt) const
{
	// #todo CmdBuffer-Signal 구조 생각해보기
    std::vector<VkSemaphoreSubmitInfo> waitSemaphoreSubmitInfos;
    waitSemaphoreSubmitInfos.resize(waitSemaphores.size());
    std::transform(waitSemaphores.begin(), waitSemaphores.end(),
                   waitSemaphoreSubmitInfos.begin(),
                   [waitAt](const CRef<Semaphore> semaphoreRef) {
                       const Semaphore& semaphore = semaphoreRef.get();
                       return VkSemaphoreSubmitInfo{
                           .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
                           .pNext = nullptr,
                           .semaphore = semaphore.GetNative(),
                           .value = semaphore.GetCurrentValue(),
                           .stageMask = waitAt,
                           .deviceIndex = 0};
                   });

    std::vector<VkSemaphoreSubmitInfo> signalSemaphoreSubmitInfos;
    signalSemaphoreSubmitInfos.resize(signalSemaphores.size());
    std::transform(signalSemaphores.begin(), signalSemaphores.end(),
                   signalSemaphoreSubmitInfos.begin(),
                   [signalAt](const Ref<Semaphore> semaphoreRef) {
                       Semaphore& semaphore = semaphoreRef.get();
                       semaphore.IncrementValue();
                       return VkSemaphoreSubmitInfo{
                           .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
                           .pNext = nullptr,
                           .semaphore = semaphore.GetNative(),
                           .value = semaphore.GetCurrentValue(),
                           .stageMask = signalAt,
                           .deviceIndex = 0};
                   });

    std::vector<VkCommandBufferSubmitInfo> cmdBufferSubmitInfos;
    cmdBufferSubmitInfos.resize(cmdBuffers.size());
    std::transform(cmdBuffers.begin(), cmdBuffers.end(),
                   cmdBufferSubmitInfos.begin(),
                   [](const CRef<CommandBuffer> cmdBufferRef) {
                       const CommandBuffer& cmdBuffer = cmdBufferRef.get();
                       return VkCommandBufferSubmitInfo{
                           .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
                           .pNext = nullptr,
                           .commandBuffer = cmdBuffer.GetNative(),
                           .deviceMask = 0};
                   });

    const VkSubmitInfo2 submitInfo{
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
        .pNext = nullptr,
        .waitSemaphoreInfoCount = static_cast<uint32_t>(waitSemaphoreSubmitInfos.size()),
        .pWaitSemaphoreInfos = waitSemaphoreSubmitInfos.data(),
        .commandBufferInfoCount =  static_cast<uint32_t>(cmdBufferSubmitInfos.size()),
        .pCommandBufferInfos = cmdBufferSubmitInfos.data(),
        .signalSemaphoreInfoCount =  static_cast<uint32_t>(signalSemaphoreSubmitInfos.size()),
        .pSignalSemaphoreInfos = signalSemaphoreSubmitInfos.data()};

    vkQueueSubmit2(GetQueue(queueType), 1, &submitInfo, VK_NULL_HANDLE);
}

void VulkanRHI::SubmitImmediateTo(const CommandBuffer& cmdBuffer) const
{
    const std::unique_ptr<Semaphore> temporary = std::make_unique<Semaphore>(std::format("ImmediateFence for {}", cmdBuffer.GetName()), vulkanContext);
    CRefArray<CommandBuffer, 1> cmdBuffers = {cmdBuffer};
    CRefArray<Semaphore, 1> waitSemaphores = {*temporary};
    RefArray<Semaphore, 1> signalSemaphores = {*temporary};
    SubmitSync(cmdBuffer.GetQueueType(), cmdBuffers,
               waitSemaphores, VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT,
               signalSemaphores, VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT);
    temporary->Wait();
}

void VulkanRHI::Present(const VkPresentInfoKHR& presentInfo) const
{
    const auto queue = GetQueue(EQueueType::Present);
    vkQueuePresentKHR(queue, &presentInfo);
}

void VulkanRHI::Present(const Swapchain& swapchain, const Semaphore& waitSemaphore) const
{
    const auto swapchainImageIdx = swapchain.GetCurrentImageIndex();
    const auto waitSemaphoreNative = waitSemaphore.GetNative();
    const auto swapchainNative = swapchain.GetNative();
    const VkPresentInfoKHR presentInfo{
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .pNext = nullptr,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &waitSemaphoreNative,
        .swapchainCount = 1,
        .pSwapchains = &swapchainNative,
        .pImageIndices = &swapchainImageIdx};

    Present(presentInfo);
}

void VulkanRHI::WaitQueueForIdle(const EQueueType queueType) const
{
    const VkQueue queue = GetQueue(queueType);
    vkQueueWaitIdle(queue);
}

void VulkanRHI::WaitAllQueuesForIdle() const
{
    vkQueueWaitIdle(graphicsQueue);
    vkQueueWaitIdle(computeQueue);
    vkQueueWaitIdle(transferQueue);
    vkQueueWaitIdle(presentQueue);
}

void VulkanRHI::WaitForDeviceIdle() const
{
    vkDeviceWaitIdle(device);
}

size_t VulkanRHI::PadUniformBufferSize(const size_t allocSize) const
{
    return PadSizeWithAlignment(allocSize, gpuProperties.limits.minUniformBufferOffsetAlignment);
}

size_t VulkanRHI::PadStorageBufferSize(size_t allocSize) const
{
    return PadSizeWithAlignment(allocSize, gpuProperties.limits.minStorageBufferOffsetAlignment);
}

void* VulkanRHI::Map(const Buffer& buffer) const
{
    void* data;
    vmaMapMemory(allocator, buffer.GetAllocation(), &data);
    return data;
}

void VulkanRHI::Unmap(const Buffer& buffer) const
{
    vmaUnmapMemory(allocator, buffer.GetAllocation());
}

void* VulkanRHI::Map(const Texture& texture) const
{
    void* data;
    vmaMapMemory(allocator, texture.GetAllocation(), &data);
    return data;
}

void VulkanRHI::Unmap(const Texture& texture) const
{
    vmaUnmapMemory(allocator, texture.GetAllocation());
}

void VulkanRHI::SetObjectName(const uint64_t object, const VkObjectType objectType, const std::string_view name) const
{
#if defined(DEBUG) || defined(_DEBUG)
    if (!name.empty())
    {
        const VkDebugUtilsObjectNameInfoEXT nameInfo{
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
            .pNext = nullptr,
            .objectType = objectType,
            .objectHandle = object,
            .pObjectName = name.data()};

        VK_ASSERT(vkSetDebugUtilsObjectNameEXT(device, &nameInfo), "Failed to set object name {}", name);
    }
#endif
}

void VulkanRHI::InitQueues(const vkb::Device& vkbDevice)
{
    spdlog::trace("Initializing queues..");
    const auto graphicsQueueRes = vkbDevice.get_queue(vkb::QueueType::graphics);
    SY_ASSERT(graphicsQueueRes.has_value(), "Failed to get graphics queue from logical device of vulkan.");
    graphicsQueue = graphicsQueueRes.value();
    graphicsQueueFamilyIdx = vkbDevice.get_queue_index(vkb::QueueType::graphics).value();
    spdlog::trace("Graphics Queue successfully acquired. Family Index: {}.", graphicsQueueFamilyIdx);

    const auto computeQueueRes = vkbDevice.get_queue(vkb::QueueType::compute);
    SY_ASSERT(computeQueueRes.has_value(), "Failed to get compute queue from logical device of vulkan.");
    computeQueue = computeQueueRes.value();
    computeQueueFamilyIdx = vkbDevice.get_queue_index(vkb::QueueType::compute).value();
    spdlog::trace("Compute Queue successfully acquired. Family Index: {}.", computeQueueFamilyIdx);

    const auto transferQueueRes = vkbDevice.get_queue(vkb::QueueType::transfer);
    SY_ASSERT(transferQueueRes.has_value(), "Failed to get transfer queue from logical device of vulkan.");
    transferQueue = transferQueueRes.value();
    transferQueueFamilyIdx = vkbDevice.get_queue_index(vkb::QueueType::transfer).value();
    spdlog::trace("Transfer Queue successfully acquired. Family Index: {}.", transferQueueFamilyIdx);

    const auto presentQueueRes = vkbDevice.get_queue(vkb::QueueType::present);
    SY_ASSERT(presentQueueRes.has_value(), "Failed to get present queue from logical device of vulkan.");
    presentQueue = presentQueueRes.value();
    presentQueueFamilyIdx = vkbDevice.get_queue_index(vkb::QueueType::present).value();
    spdlog::trace("Present Queue successfully acquired. Family Index: {}.", presentQueueFamilyIdx);
}

bool VulkanRHI::IsFormatSupportFeatures(VkFormat format, VkFormatFeatureFlagBits2 featureFlag, bool bIsOptimalTiling /*= true*/) const
{
    VkFormatProperties2 props;
    props.sType = VK_STRUCTURE_TYPE_FORMAT_PROPERTIES_2;
    props.pNext = nullptr;
    vkGetPhysicalDeviceFormatProperties2(physicalDevice, format, &props);
    return ContainsBitFlag<uint64_t>(
        bIsOptimalTiling ? props.formatProperties.optimalTilingFeatures : props.formatProperties.linearTilingFeatures,
        featureFlag);
}
} // namespace sy::vk
