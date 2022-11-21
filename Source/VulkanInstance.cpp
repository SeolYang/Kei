#include <Core.h>
#include <VulkanInstance.h>
#include <Window.h>

namespace sy
{
	VulkanInstance::VulkanInstance(Window& window) :
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

	void VulkanInstance::Startup()
	{
		volkInitialize();

		vkb::InstanceBuilder instanceBuilder;
		auto instanceBuilderRes = instanceBuilder.set_app_name(window.GetWindowTitle().data())
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
			//.add_required_extension("VK_KHR_depth_stencil_resolve")
			//.add_required_extension("VK_KHR_get_physical_device_properties2")
			//.add_required_extension("VK_KHR_dynamic_rendering")
			.select()
			.value();

		physicalDevice = vkbPhysicalDevice.physical_device;
		gpuName = vkbPhysicalDevice.properties.deviceName;

		vkb::DeviceBuilder deviceBuilder{ vkbPhysicalDevice };
		VkPhysicalDeviceShaderDrawParametersFeatures shaderDrawParameters
		{
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_DRAW_PARAMETERS_FEATURES,
			.pNext = nullptr,
			.shaderDrawParameters = VK_TRUE
		};

		VkPhysicalDeviceDynamicRenderingFeaturesKHR dynamicRenderingFeatures
		{
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES_KHR,
			.pNext = nullptr,
			.dynamicRendering = VK_TRUE
		};

		auto vkbDeviceRes = deviceBuilder.add_pNext(&shaderDrawParameters)
			.add_pNext(&dynamicRenderingFeatures)
			.build();
		SY_ASSERT(vkbDeviceRes.has_value(), "Failed to create device using GPU {}.", gpuName);
		auto vkbDevice = vkbDeviceRes.value();
		device = vkbDevice.device;
		spdlog::trace("Succeed to create logical device using GPU {}.", gpuName);

		const VmaVulkanFunctions vulkanFunctions
		{
			.vkGetInstanceProcAddr = vkGetInstanceProcAddr,
			.vkGetDeviceProcAddr = vkGetDeviceProcAddr
		};

		const VmaAllocatorCreateInfo allocatorInfo
		{
			.physicalDevice = physicalDevice,
			.device = device,
			.pVulkanFunctions = &vulkanFunctions,
			.instance = instance,
		};
		VK_ASSERT(vmaCreateAllocator(&allocatorInfo, &allocator), "Failed to create vulkan memory allocator instance.");
		spdlog::trace("Vulkan memory allocator instance successfully created.");

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

	void VulkanInstance::Cleanup()
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
