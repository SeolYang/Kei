#pragma once
#include <Core/Core.h>

namespace sy
{
	namespace vk
	{
		class VulkanContext;
		class CommandBuffer;
		class Fence;
		class CommandPool : public VulkanWrapper<VkCommandPool>
		{
		public:
			struct Deallocation
			{
				const OffsetPool::Slot_t slot;
			};

			using CommandBufferAllocation = std::unique_ptr<CommandBuffer, std::function<void(CommandBuffer*)>>;

		public:
			CommandPool(const VulkanContext& vulkanContext, EQueueType queueType);
			virtual ~CommandPool() override = default;

			CommandBufferAllocation RequestCommandBuffer(std::string_view name);
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
}
