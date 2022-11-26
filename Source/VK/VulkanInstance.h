#pragma once
#include <Core.h>

namespace sy
{
	class Window;
	class Swapchain;
	class CommandBuffer;
	class CommandPool;
	class Fence;
	class Semaphore;
	class VulkanInstance
	{
	public:
		VulkanInstance(const Window& window);
		~VulkanInstance();

		[[nodiscard]] CommandPool& RequestGraphicsCommandPool() { return RequestCommandPool(EQueueType::Graphics, graphicsCmdPools, graphicsCmdPoolListMutex); }
		[[nodiscard]] CommandPool& RequestComputeCommandPool() { return RequestCommandPool(EQueueType::Compute, computeCmdPools, computeCmdPoolListMutex); }
		[[nodiscard]] CommandPool& RequestTransferCommandPool() { return RequestCommandPool(EQueueType::Transfer, transferCmdPools, transferCmdPoolListMutex); }
		[[nodiscard]] CommandPool& RequestPresentCommandPool() { return RequestCommandPool(EQueueType::Present, presentCmdPools, presentCmdPoolListMutex); }

		[[nodiscard]] VkPhysicalDevice GetPhysicalDevice() const { return physicalDevice; }
		[[nodiscard]] VkDevice GetLogicalDevice() const { return device; }
		[[nodiscard]] uint32_t GetQueueFamilyIndex(EQueueType queueType) const;
		[[nodiscard]] VkQueue GetQueue(EQueueType queueType) const;
		[[nodiscard]] VkSurfaceKHR GetSurface() const { return surface; }
		[[nodiscard]] Swapchain& GetSwapchain() const { return *swapchain; }

		void SubmitTo(EQueueType queueType, const VkSubmitInfo& submitInfo, const Fence& fence) const;
		void SubmitTo(
			EQueueType queueType, 
			std::span<std::reference_wrapper<const Semaphore>> waitSemaphores,
			std::span<std::reference_wrapper<const CommandBuffer>> cmdBuffers,
			std::span<std::reference_wrapper<const Semaphore>> signalSemaphores,
			VkPipelineStageFlags waitStage, const Fence& fence) const;

		void Present(const VkPresentInfoKHR& presentInfo) const;
		void Present(const Swapchain& swapchain, const Semaphore& waitSemaphore) const;

		void WaitQueueForIdle(EQueueType queueType) const;
		void WaitAllQueuesForIdle() const;

	private:
		void Startup();
		void Cleanup();

		void InitCommandPools(const vkb::Device& vkbDevice);

		[[nodiscard]] CommandPool& RequestCommandPool(EQueueType queueType, std::vector<std::unique_ptr<CommandPool>>& poolList, std::mutex& listMutex);

	private:
		const Window& window;
		VkInstance instance;
		VkDebugUtilsMessengerEXT debugMessenger;
		VkSurfaceKHR surface;
		VkPhysicalDevice physicalDevice;
		VkDevice device;
		std::string gpuName;

		std::unique_ptr<Swapchain> swapchain;

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