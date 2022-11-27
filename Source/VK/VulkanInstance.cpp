#include <Core.h>
#include <Window.h>
#include <VK/VulkanInstance.h>
#include <VK/Swapchain.h>
#include <VK/CommandBuffer.h>
#include <VK/CommandPool.h>
#include <VK/Semaphore.h>
#include <VK/Fence.h>
#include <VK/DescriptorPool.h>

namespace sy
{
	VulkanInstance::VulkanInstance(const Window& window) :
		window(window),
		instance(VK_NULL_HANDLE),
		surface(VK_NULL_HANDLE),
		physicalDevice(VK_NULL_HANDLE),
		device(VK_NULL_HANDLE),
		allocator(VK_NULL_HANDLE)
	{
		Startup();
	}

	VulkanInstance::~VulkanInstance()
	{
		Cleanup();
	}

	uint32_t VulkanInstance::GetQueueFamilyIndex(const EQueueType queueType) const
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

	VkQueue VulkanInstance::GetQueue(EQueueType queueType) const
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

	void VulkanInstance::SubmitTo(const EQueueType queueType, const VkSubmitInfo& submitInfo, const Fence& fence) const
	{
		const auto queue = GetQueue(queueType);
		SY_ASSERT(queue != VK_NULL_HANDLE, "Invalid queue submission request.");

		VK_ASSERT(vkQueueSubmit(queue, 1, &submitInfo, fence.GetNativeHandle()), "Failed to submit to queue.");
	}

	void VulkanInstance::SubmitTo(EQueueType queueType, const std::span<std::reference_wrapper<const Semaphore>> waitSemaphores, const std::span<std::reference_wrapper<const CommandBuffer>> cmdBuffers, std::span<std::reference_wrapper<const Semaphore>> signalSemaphores, const VkPipelineStageFlags waitStage, const Fence& fence) const
	{
		const auto waitSemaphoreNatives = TransformVulkanWrappersToNatives(waitSemaphores);
		const auto cmdBufferNatives = TransformVulkanWrappersToNativesWithValidation<CommandBuffer>(cmdBuffers
			, [queueType](const CRef<CommandBuffer> cmdBufferRef)
			{
				const auto& cmdBuffer = cmdBufferRef.get();
				return cmdBuffer.GetQueueType() == queueType;
			});
		const auto signalSemaphoreNatives = TransformVulkanWrappersToNatives(signalSemaphores);
		const VkSubmitInfo submitInfo
		{
			.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
			.pNext = nullptr,
			.waitSemaphoreCount = static_cast<uint32_t>(waitSemaphoreNatives.size()),
			.pWaitSemaphores = waitSemaphoreNatives.data(),
			.pWaitDstStageMask = &waitStage,
			.commandBufferCount = static_cast<uint32_t>(cmdBufferNatives.size()),
			.pCommandBuffers = cmdBufferNatives.data(),
			.signalSemaphoreCount = static_cast<uint32_t>(signalSemaphoreNatives.size()),
			.pSignalSemaphores = signalSemaphoreNatives.data()
		};

		SubmitTo(queueType, submitInfo, fence);
	}

	void VulkanInstance::Present(const VkPresentInfoKHR& presentInfo) const
	{
		const auto queue = GetQueue(EQueueType::Present);
		vkQueuePresentKHR(queue, &presentInfo);
	}

	void VulkanInstance::Present(const Swapchain& swapchain, const Semaphore& waitSemaphore) const
	{
		const auto swapchainImageIdx = swapchain.GetCurrentImageIndex();
		const auto waitSemaphoreNative = waitSemaphore.GetNativeHandle();
		const auto swapchainNative = swapchain.GetNativeHandle();
		const VkPresentInfoKHR presentInfo
		{
			.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
			.pNext = nullptr,
			.waitSemaphoreCount = 1,
			.pWaitSemaphores = &waitSemaphoreNative,
			.swapchainCount = 1,
			.pSwapchains = &swapchainNative,
			.pImageIndices = &swapchainImageIdx
		};

		Present(presentInfo);
	}

	void VulkanInstance::WaitQueueForIdle(const EQueueType queueType) const
	{
		const VkQueue queue = GetQueue(queueType);
		vkQueueWaitIdle(queue);
	}

	void VulkanInstance::WaitAllQueuesForIdle() const
	{
		vkQueueWaitIdle(graphicsQueue);
		vkQueueWaitIdle(computeQueue);
		vkQueueWaitIdle(transferQueue);
		vkQueueWaitIdle(presentQueue);
	}

	void VulkanInstance::BeginFrame(const size_t currentInFlightFrameIdx) const
	{
		const auto& frameDependCmdPools = cmdPools[currentInFlightFrameIdx];
		for (const auto& cmdPool : frameDependCmdPools)
		{
			cmdPool->BeginFrame();
		}
	}

	void VulkanInstance::Startup()
	{
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

		vkb::PhysicalDeviceSelector physicalDeviceSelector { vkbInstance };
		auto vkbPhysicalDevice = physicalDeviceSelector.set_minimum_version(1, 3)
			.set_surface(surface)
			.add_required_extension("VK_EXT_descriptor_indexing")
			.add_required_extension("VK_KHR_swapchain")
			.add_required_extension("VK_KHR_dynamic_rendering")
			.select()
			.value();

		physicalDevice = vkbPhysicalDevice.physical_device;
		gpuName = vkbPhysicalDevice.properties.deviceName;

		vkb::DeviceBuilder deviceBuilder{ vkbPhysicalDevice };
		VkPhysicalDeviceDynamicRenderingFeaturesKHR dynamicRenderingFeatures
		{
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES_KHR,
			.pNext = nullptr,
			.dynamicRendering = VK_TRUE
		};

		VkPhysicalDeviceDescriptorIndexingFeatures descriptorIndexingFeatures
		{
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

		auto vkbDeviceRes = 
			deviceBuilder.add_pNext(&dynamicRenderingFeatures)
			.add_pNext(&descriptorIndexingFeatures)
			.build();
		SY_ASSERT(vkbDeviceRes.has_value(), "Failed to create device using GPU {}.", gpuName);
		auto& vkbDevice = vkbDeviceRes.value();
		device = vkbDevice.device;
		spdlog::trace("Succeed to create logical device using GPU {}.", gpuName);

		swapchain = std::make_unique<Swapchain>(window , *this);

		const VmaVulkanFunctions vkFunctions
		{
			.vkGetInstanceProcAddr = vkGetInstanceProcAddr,
			.vkGetDeviceProcAddr = vkGetDeviceProcAddr
		};

		const VmaAllocatorCreateInfo allocatorInfo
		{
			.physicalDevice = physicalDevice,
			.device = device,
			.pVulkanFunctions = &vkFunctions,
			.instance = instance,
		};
		VK_ASSERT(vmaCreateAllocator(&allocatorInfo, &allocator), "Failed to create vulkan memory allocator instance.");
		spdlog::trace("VMA instance successfully created.");

		InitQueues(vkbDevice);

	}

	void VulkanInstance::Cleanup()
	{
		WaitAllQueuesForIdle();
		{
			spdlog::trace("Cleanup command pools...");
			{
				RWLock lock(cmdPoolMutex);
				for (auto& cmdPoolVec : cmdPools)
				{
					cmdPoolVec.clear();
				}
			}
			allocator = VK_NULL_HANDLE;

			spdlog::trace("Cleanup swap chain...");
			swapchain.reset();

			spdlog::trace("Cleanup vulkan logical device...");
			vkDestroyDevice(device, nullptr);
			device = VK_NULL_HANDLE;
			spdlog::trace("Cleanup vulkan surface...");
			vkDestroySurfaceKHR(instance, surface, nullptr);
			surface = VK_NULL_HANDLE;
			vkb::destroy_debug_utils_messenger(instance, debugMessenger, nullptr);
			debugMessenger = VK_NULL_HANDLE;
			spdlog::trace("Cleanup vulkan instance...");
			vkDestroyInstance(instance, nullptr);
			instance = VK_NULL_HANDLE;
		}
	}

	void VulkanInstance::InitQueues(const vkb::Device& vkbDevice)
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
		transferQueue = computeQueueRes.value();
		transferQueueFamilyIdx = vkbDevice.get_queue_index(vkb::QueueType::transfer).value();
		spdlog::trace("Transfer Queue successfully acquired. Family Index: {}.", transferQueueFamilyIdx);

		const auto presentQueueRes = vkbDevice.get_queue(vkb::QueueType::present);
		SY_ASSERT(presentQueueRes.has_value(), "Failed to get present queue from logical device of vulkan.");
		presentQueue = presentQueueRes.value();
		presentQueueFamilyIdx = vkbDevice.get_queue_index(vkb::QueueType::present).value();
		spdlog::trace("Present Queue successfully acquired. Family Index: {}.", presentQueueFamilyIdx);
	}

	CommandPool& VulkanInstance::RequestCommandPool(const EQueueType queueType, const size_t currentInFlightFrameIdx)
	{
		thread_local robin_hood::unordered_map<EQueueType, std::array<CommandPool*, NumMaxInFlightFrames>> localCmdPools;
		if (!localCmdPools.contains(queueType))
		{
			RWLock lock(cmdPoolMutex);
			for (size_t inFlightFrameIdx = 0; inFlightFrameIdx < NumMaxInFlightFrames; ++inFlightFrameIdx)
			{
				auto* newCmdPool = new CommandPool(*this, queueType);
				localCmdPools[queueType][inFlightFrameIdx] = newCmdPool;
				cmdPools[inFlightFrameIdx].emplace_back(newCmdPool);
			}
		}

		return *(localCmdPools[queueType][currentInFlightFrameIdx]);
	}
}
