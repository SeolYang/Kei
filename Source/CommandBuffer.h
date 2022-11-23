#pragma once
#include <Core.h>

namespace sy
{
	class VulkanInstance;
	class CommandPool;
	class Fence;
	class CommandBuffer : public NamedType
	{
	public:
		CommandBuffer(std::string_view name, const VulkanInstance& vulkanInstance, const CommandPool& cmdPool);
		virtual ~CommandBuffer() override = default;

		[[nodiscard]] VkCommandBuffer GetCommandBuffer() const { return cmdBuffer; }
		[[nodiscard]] EQueueType GetQueueType() const { return queueType; }

		[[nodiscard]] bool IsReadyToUse() const;

		const CommandBuffer& ResetFence() const;
		const CommandBuffer& Begin() const;
		const CommandBuffer& End() const;

	private:
		const EQueueType queueType;
		const VulkanInstance& vulkanInstance;
		const CommandPool& cmdPool;
		VkCommandBuffer cmdBuffer;
		std::unique_ptr<Fence> fence;

	};

}
