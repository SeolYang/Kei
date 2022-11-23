#pragma once
#include <Core.h>

namespace sy
{
	class VulkanInstance;
	class CommandBuffer;
	class CommandPool : public NamedType
	{
	public:
		CommandPool(const VulkanInstance& vulkanInstance, EQueueType queueType);
		virtual ~CommandPool() override;

		const CommandBuffer& RequestCommandBuffer(std::string_view name);
		[[nodiscard]] EQueueType GetQueueType() const { return queueType; }
		[[nodiscard]] VkCommandPool GetCommandPool() const { return pool; }

	private:
		const VulkanInstance& vulkanInstance;
		const EQueueType queueType;
		VkCommandPool pool;

		std::vector<std::unique_ptr<CommandBuffer>> cmdBuffers;

	};


}
