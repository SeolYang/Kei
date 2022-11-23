#pragma once
#include <Core.h>

namespace sy
{
	class Window;
	class CommandPool;
	class VulkanInstance
	{
	public:
		VulkanInstance(Window& window);
		~VulkanInstance();

		[[nodiscard]] CommandPool& RequestGraphicsCommandPool() { return RequestCommandPool(EQueueType::Graphics, graphicsCmdPools, graphicsCmdPoolListMutex); }
		[[nodiscard]] CommandPool& RequestComputeCommandPool() { return RequestCommandPool(EQueueType::Compute, computeCmdPools, computeCmdPoolListMutex); }
		[[nodiscard]] CommandPool& RequestTransferCommandPool() { return RequestCommandPool(EQueueType::Transfer, transferCmdPools, transferCmdPoolListMutex); }
		[[nodiscard]] CommandPool& RequestPresentCommandPool() { return RequestCommandPool(EQueueType::Present, presentCmdPools, presentCmdPoolListMutex); }

		[[nodiscard]] VkPhysicalDevice GetPhysicalDevice() const { return physicalDevice; }
		[[nodiscard]] VkDevice GetLogicalDevice() const { return device; }
		[[nodiscard]] uint32_t GetQueueFamilyIndex(EQueueType queue) const;
		[[nodiscard]] VkSurfaceKHR GetSurface() const { return surface; }

	private:
		void Startup();
		void Cleanup();

		void InitCommandPools(vkb::Device& vkbDevice);

		[[nodiscard]] CommandPool& RequestCommandPool(EQueueType queueType, std::vector<std::unique_ptr<CommandPool>>& poolList, std::mutex& listMutex);

	private:
		Window& window;
		VkInstance instance;
		VkDebugUtilsMessengerEXT debugMessenger;
		VkSurfaceKHR surface;
		VkPhysicalDevice physicalDevice;
		VkDevice device;
		std::string gpuName;

		VmaAllocator allocator;

		VkQueue graphicsQueue;
		VkQueue computeQueue;
		VkQueue transferQueue;
		VkQueue presentQueue;

		uint32_t graphicsQueueFamilyIdx;
		uint32_t computeQueueFamilyIdx;
		uint32_t transferQueueFamilyIdx;
		uint32_t presentQueueFamilyIdx;

		std::vector<std::unique_ptr<CommandPool>> graphicsCmdPools;
		std::mutex graphicsCmdPoolListMutex;
		std::vector<std::unique_ptr<CommandPool>> computeCmdPools;
		std::mutex computeCmdPoolListMutex;
		std::vector<std::unique_ptr<CommandPool>> transferCmdPools;
		std::mutex transferCmdPoolListMutex;
		std::vector<std::unique_ptr<CommandPool>> presentCmdPools;
		std::mutex presentCmdPoolListMutex;

	};
}