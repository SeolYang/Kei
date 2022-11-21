#pragma once
#include <Core.h>

namespace sy
{
	class Window;
	class VulkanInstance
	{
	public:
		VulkanInstance(Window& window);
		~VulkanInstance();

	private:
		void Startup();
		void Cleanup();

	private:
		Window& window;
		VkInstance instance;
		VkDebugUtilsMessengerEXT debugMessenger;
		VkSurfaceKHR surface;
		VkPhysicalDevice physicalDevice;
		VkDevice device;

		VmaAllocator allocator;

	};
}