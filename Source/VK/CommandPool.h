#pragma once
#include <PCH.h>

namespace sy::vk
{
	class VulkanRHI;
	class CommandBuffer;
	class Fence;
	class CommandPool : public VulkanWrapper<VkCommandPool>
	{
	public:
		struct Deallocation
		{
			const OffsetPool::Slot_t slot;
		};

	public:
		CommandPool(const VulkanRHI& vulkanRHI, EQueueType queueType);
		virtual ~CommandPool() override = default;

		ManagedCommandBuffer RequestCommandBuffer(std::string_view name);
		[[nodiscard]] EQueueType GetQueueType() const { return queueType; }
		void BeginFrame();

		void Reset() const;

	private:
		const EQueueType queueType;
		OffsetPool offsetPool;
		std::vector<std::unique_ptr<CommandBuffer>> cmdBuffers;
		std::vector<Deallocation> pendingDeallocations;
	};
}
