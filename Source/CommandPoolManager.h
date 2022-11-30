#pragma once
#include <Core.h>

namespace sy
{
	class VulkanContext;
	class FrameTracker;
	class CommandPool;
	class CommandBuffer;
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