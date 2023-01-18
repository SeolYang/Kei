#include <PCH.h>
#include <Core/Window.h>
#include <VK/VulkanContext.h>
#include <VK/Swapchain.h>
#include <VK/CommandBuffer.h>
#include <VK/CommandPool.h>
#include <VK/Semaphore.h>
#include <VK/Fence.h>
#include <VK/Buffer.h>
#include <VK/FrameTracker.h>

namespace sy
{
	namespace vk
	{
		VulkanContext::VulkanContext(const Window& window) :
			window(window),
			instance(VK_NULL_HANDLE),
			surface(VK_NULL_HANDLE),
			physicalDevice(VK_NULL_HANDLE),
			device(VK_NULL_HANDLE),
			allocator(VK_NULL_HANDLE)
		{
			Startup();
		}

		VulkanContext::~VulkanContext()
		{
			Cleanup();
		}

		uint32_t VulkanContext::GetQueueFamilyIndex(const EQueueType queueType) const
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

		VkQueue VulkanContext::GetQueue(const EQueueType queueType) const
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

		void VulkanContext::SubmitImmediateTo(const CommandBuffer& cmdBuffer) const
		{
			SubmitTo(cmdBuffer, *immediateFence);
			immediateFence->Wait();
			immediateFence->Reset();
		}

		void VulkanContext::SubmitTo(const EQueueType queueType, const VkSubmitInfo& submitInfo, const Fence& fence) const
		{
			const auto queue = GetQueue(queueType);
			SY_ASSERT(queue != VK_NULL_HANDLE, "Invalid queue submission request.");

			VK_ASSERT(vkQueueSubmit(queue, 1, &submitInfo, fence.GetNativeHandle()), "Failed to submit to queue.");
		}

		void VulkanContext::SubmitTo(const CommandBuffer& cmdBuffer, const Fence& fence) const
		{
			const auto cmdBufferHandle = cmdBuffer.GetNativeHandle();
			const VkSubmitInfo submitInfo
			{
				.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
				.pNext = nullptr,
				.waitSemaphoreCount = 0,
				.pWaitSemaphores = nullptr,
				.pWaitDstStageMask = nullptr,
				.commandBufferCount = 1,
				.pCommandBuffers = &cmdBufferHandle,
				.signalSemaphoreCount = 0,
				.pSignalSemaphores = nullptr
			};

			SubmitTo(cmdBuffer.GetQueueType(), submitInfo, fence);
		}

		void VulkanContext::SubmitTo(const EQueueType queueType, const FrameTracker& frameTracker, const std::span<CRef<CommandBuffer>> cmdBuffers)
		{
			const auto& renderFence = frameTracker.GetCurrentInFlightRenderFence();
			auto& presentSemaphore = frameTracker.GetCurrentInFlightPresentSemaphore();
			const auto& renderSemaphore = frameTracker.GetCurrentInFlightRenderSemaphore();

			CRefVec<vk::Semaphore> waitSemaphores;
			waitSemaphores.emplace_back(presentSemaphore);
			CRefVec<vk::Semaphore> signalSemaphores;
			signalSemaphores.emplace_back(renderSemaphore);

			SubmitTo(queueType,
				waitSemaphores,
				cmdBuffers,
				signalSemaphores,
				VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, renderFence);
		}

		void VulkanContext::SubmitTo(EQueueType queueType, const std::span<std::reference_wrapper<const Semaphore>> waitSemaphores, const std::span<std::reference_wrapper<const CommandBuffer>> cmdBuffers, std::span<std::reference_wrapper<const Semaphore>> signalSemaphores, const VkPipelineStageFlags waitStage, const Fence& fence) const
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

		void VulkanContext::Present(const VkPresentInfoKHR& presentInfo) const
		{
			const auto queue = GetQueue(EQueueType::Present);
			vkQueuePresentKHR(queue, &presentInfo);
		}

		void VulkanContext::Present(const Swapchain& swapchain, const Semaphore& waitSemaphore) const
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

		void VulkanContext::WaitQueueForIdle(const EQueueType queueType) const
		{
			const VkQueue queue = GetQueue(queueType);
			vkQueueWaitIdle(queue);
		}

		void VulkanContext::WaitAllQueuesForIdle() const
		{
			vkQueueWaitIdle(graphicsQueue);
			vkQueueWaitIdle(computeQueue);
			vkQueueWaitIdle(transferQueue);
			vkQueueWaitIdle(presentQueue);
		}

		void VulkanContext::WaitForDeviceIdle() const
		{
			vkDeviceWaitIdle(device);
		}

		size_t VulkanContext::PadUniformBufferSize(const size_t allocSize) const
		{
			return PadSizeWithAlignment(allocSize, gpuProperties.limits.minUniformBufferOffsetAlignment);
		}

		size_t VulkanContext::PadStorageBufferSize(size_t allocSize) const
		{
			return PadSizeWithAlignment(allocSize, gpuProperties.limits.minStorageBufferOffsetAlignment);
		}

		void* VulkanContext::Map(const Buffer& buffer) const
		{
			void* data;
			vmaMapMemory(allocator, buffer.GetAllocation(), &data);
			return data;
		}

		void VulkanContext::Unmap(const Buffer& buffer) const
		{
			vmaUnmapMemory(allocator, buffer.GetAllocation());
		}

		void VulkanContext::SetObjectName(const uint64_t object, const VkObjectType objectType, const std::string_view name) const
		{
#if defined(DEBUG) || defined(_DEBUG)
			if (!name.empty())
			{
				const VkDebugUtilsObjectNameInfoEXT nameInfo
				{
					.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
					.pNext = nullptr,
					.objectType = objectType,
					.objectHandle = object,
					.pObjectName = name.data()
				};

				VK_ASSERT(vkSetDebugUtilsObjectNameEXT(device, &nameInfo), "Failed to set object name {}", name);
			}
#endif
		}

		void VulkanContext::Startup()
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

			vkb::PhysicalDeviceSelector physicalDeviceSelector{ vkbInstance };
			auto vkbPhysicalDevice = physicalDeviceSelector.set_minimum_version(1, 3)
				.set_surface(surface)
				.add_required_extension(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME)
				.add_required_extension(VK_KHR_SWAPCHAIN_EXTENSION_NAME)
				.add_required_extension(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME)
				.add_required_extension(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME)
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

			VkPhysicalDeviceSynchronization2Features synchronization2Features
			{
				.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES,
				.pNext = nullptr,
				.synchronization2 = true
			};

			auto vkbDeviceRes =
				deviceBuilder.add_pNext(&dynamicRenderingFeatures)
				.add_pNext(&descriptorIndexingFeatures)
				.add_pNext(&synchronization2Features)
				.build();
			SY_ASSERT(vkbDeviceRes.has_value(), "Failed to create device using GPU {}.", gpuName);
			auto& vkbDevice = vkbDeviceRes.value();
			device = vkbDevice.device;
			spdlog::trace("Succeed to create logical device using GPU {}.", gpuName);

			swapchain = std::make_unique<Swapchain>(window, *this);

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

			immediateFence = std::make_unique<Fence>("Immediate Fence", *this, false);
		}

		void VulkanContext::Cleanup()
		{
			WaitAllQueuesForIdle();
			{
				immediateFence.reset();
				vmaDestroyAllocator(allocator);
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

		void VulkanContext::InitQueues(const vkb::Device& vkbDevice)
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
	}
}
