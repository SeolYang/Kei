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
		auto features = vkbPhysicalDevice.features;
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

		vkb::Device vkbDevice = deviceBuilder.add_pNext(&shaderDrawParameters)
		.add_pNext(&dynamicRenderingFeatures)
		.build().value();
		device = vkbDevice.device;

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
			.instance = instance
		};
		vmaCreateAllocator(&allocatorInfo, &allocator);
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
