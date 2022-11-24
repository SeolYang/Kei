#pragma once
#include <Core.h>

namespace sy
{
	class VulkanInstance;
	class CommandPool;
	class Fence;
	class CommandBuffer : public VulkanWrapper<VkCommandBuffer>
	{
	public:
		CommandBuffer(std::string_view name, const VulkanInstance& vulkanInstance, const CommandPool& cmdPool);
		virtual ~CommandBuffer() override = default;

		[[nodiscard]] EQueueType GetQueueType() const { return queueType; }

		[[nodiscard]] bool IsReadyToUse() const;

		void ResetFence() const;
		void Begin() const;
		void End() const;

	private:
		const EQueueType queueType;
		const CommandPool& cmdPool;
		std::unique_ptr<Fence> fence;

	};

}
