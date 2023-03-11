#pragma once
#include <PCH.h>

namespace sy::vk
{
	class VulkanRHI;
	class CommandPool;
	class CommandBuffer;
	class FrameTracker;
	class CommandPoolManager final : public NonCopyable
	{
	public:
		CommandPoolManager(const VulkanRHI& vulkanRHI, const FrameTracker& frameTracker);
		virtual ~CommandPoolManager() override;

		[[nodiscard]] CommandPool& RequestCommandPool(EQueueType queueType);

		void BeginFrame();
		void EndFrame();

	private:
		const VulkanRHI& vulkanRHI;
		const FrameTracker& frameTracker;

		std::shared_mutex cmdPoolMutex;
		std::array<std::vector<std::unique_ptr<CommandPool>>, NumMaxInFlightFrames> cmdPools;

	};
}