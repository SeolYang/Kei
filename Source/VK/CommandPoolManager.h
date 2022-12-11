#pragma once
#include <Core/Core.h>

namespace sy
{
	namespace vk
	{
		class VulkanContext;
		class CommandPool;
		class CommandBuffer;
		class FrameTracker;
		class CommandPoolManager
		{
		public:
			CommandPoolManager(const VulkanContext& vulkanContext, const FrameTracker& frameTracker);
			~CommandPoolManager();

			CommandPoolManager(const CommandPoolManager&) = delete;
			CommandPoolManager(CommandPoolManager&&) = delete;
			CommandPoolManager& operator=(const CommandPoolManager&) = delete;
			CommandPoolManager& operator=(CommandPoolManager&&) = delete;

			[[nodiscard]] CommandPool& RequestCommandPool(EQueueType queueType);

			void BeginFrame();
			void EndFrame();

		private:
			const VulkanContext& vulkanContext;
			const FrameTracker& frameTracker;

			std::shared_mutex cmdPoolMutex;
			std::array<std::vector<std::unique_ptr<CommandPool>>, NumMaxInFlightFrames> cmdPools;

		};
	}
}