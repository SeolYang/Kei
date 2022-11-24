#pragma once
#include <Core.h>

namespace sy
{
	class VulkanInstance;
	class CommandBuffer;
	class CommandPool : public VulkanWrapper<VkCommandPool>
	{
	public:
		CommandPool(const VulkanInstance& vulkanInstance, EQueueType queueType);
		virtual ~CommandPool() override = default;

		CommandBuffer& RequestCommandBuffer(std::string_view name);
		[[nodiscard]] EQueueType GetQueueType() const { return queueType; }

	private:
		const EQueueType queueType;
		std::vector<std::unique_ptr<CommandBuffer>> cmdBuffers;

	};


}
