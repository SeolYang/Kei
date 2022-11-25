#pragma once
#include <Core.h>

namespace sy
{
	class VulkanInstance;
	class CommandBuffer;
	class Fence;
	class CommandPool : public VulkanWrapper<VkCommandPool>
	{
	public:
		CommandPool(const VulkanInstance& vulkanInstance, EQueueType queueType);
		virtual ~CommandPool() override = default;

		CommandBuffer& RequestCommandBuffer(std::string_view name, const Fence& renderFence);
		[[nodiscard]] EQueueType GetQueueType() const { return queueType; }

	private:
		const EQueueType queueType;
		std::vector<std::unique_ptr<CommandBuffer>> cmdBuffers;

	};


}
