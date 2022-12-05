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
	class DescriptorPool;
	class Buffer;
	class VulkanContext
	{
	public:
		VulkanContext(const Window& window);
		~VulkanContext();

		VulkanContext(const VulkanContext&) = delete;
		VulkanContext(VulkanContext&&) = delete;
		VulkanContext& operator=(const VulkanContext&) = delete;
		VulkanContext& operator=(VulkanContext&&) = delete;

		[[nodiscard]] VkPhysicalDevice GetPhysicalDevice() const { return physicalDevice; }
		[[nodiscard]] VkDevice GetDevice() const { return device; }
		[[nodiscard]] uint32_t GetQueueFamilyIndex(EQueueType queueType) const;
		[[nodiscard]] VkQueue GetQueue(EQueueType queueType) const;
		[[nodiscard]] VkSurfaceKHR GetSurface() const { return surface; }
		[[nodiscard]] Swapchain& GetSwapchain() const { return *swapchain; }
		[[nodiscard]] VmaAllocator GetAllocator() const { return allocator; }

		void SubmitTo(EQueueType queueType, const VkSubmitInfo& submitInfo, const Fence& fence) const;
		void SubmitTo(const CommandBuffer& cmdBuffer, const Fence& fence) const;
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
		void WaitForDeviceIdle() const;

		[[nodiscard]] size_t PadUniformBufferSize(size_t allocSize) const;
		[[nodiscard]] size_t PadStorageBufferSize(size_t allocSize) const;

		[[nodiscard]] void* Map(const Buffer& buffer) const;
		void Unmap(const Buffer& buffer) const;

		void SetObjectName(uint64_t object, VkObjectType objectType, std::string_view name) const;

		template <typename HandleType>
		void SetObjectName(const VulkanWrapper<HandleType>& object)
		{
			SetObjectName(reinterpret_cast<uint64_t>(object.GetNativeHandle()), object.GetType(), object.GetName());
		}

	private:
		void Startup();
		void Cleanup();

		void InitQueues(const vkb::Device& vkbDevice);

	private:
		const Window& window;
		VkInstance instance;
		VkDebugUtilsMessengerEXT debugMessenger;
		VkSurfaceKHR surface;
		VkPhysicalDevice physicalDevice;
		VkPhysicalDeviceProperties gpuProperties;
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

	};
}