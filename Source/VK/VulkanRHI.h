#pragma once
#include <PCH.h>
#include <VK/VulkanWrapper.h>

namespace sy::window
{
	class Window;
}

namespace sy::vk
{
	class Swapchain;
	class CommandBuffer;
	class CommandPool;
	class Fence;
	class Semaphore;
	class DescriptorPool;
	class Buffer;
	class FrameTracker;
	class VulkanRHI : public Subsystem
	{
	public:
		VulkanRHI(VulkanContext& vulkanContext, const window::Window& window);
		~VulkanRHI();

		void Startup() override;
		void Shutdown() override;

		[[nodiscard]] VkPhysicalDevice GetPhysicalDevice() const
		{
			return physicalDevice;
		}

		[[nodiscard]] VkDevice GetDevice() const
		{
			return device;
		}

		[[nodiscard]] uint32_t GetQueueFamilyIndex(EQueueType queueType) const;
		[[nodiscard]] VkQueue GetQueue(EQueueType queueType) const;

		[[nodiscard]] VkSurfaceKHR GetSurface() const
		{
			return surface;
		}

		[[nodiscard]] VmaAllocator GetAllocator() const
		{
			return allocator;
		}

		void SubmitImmediateTo(const CommandBuffer& cmdBuffer) const;

		void SubmitTo(EQueueType queueType, const VkSubmitInfo& submitInfo, const Fence& fence) const;
		void SubmitTo(const CommandBuffer& cmdBuffer, const Fence& fence) const;
		void SubmitTo(EQueueType queueType, const FrameTracker& frameTracker,
			std::span<CRef<CommandBuffer>> cmdBuffers) const;
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
		void InitQueues(const vkb::Device& vkbDevice);

	private:
		VulkanContext& vulkanContext;
		const window::Window& window;
		VkInstance instance;
		VkDebugUtilsMessengerEXT debugMessenger;
		VkSurfaceKHR surface;
		VkPhysicalDevice physicalDevice;
		VkPhysicalDeviceProperties gpuProperties;
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
	};
} // namespace sy::vk
