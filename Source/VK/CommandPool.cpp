#include <Core.h>
#include <VK/CommandPool.h>
#include <VK/CommandBuffer.h>
#include <VK/VulkanInstance.h>

namespace sy
{
	CommandPool::CommandPool(const VulkanInstance& vulkanInstance, const EQueueType queueType) :
		VulkanWrapper<VkCommandPool>("Unknown Pool", vulkanInstance, VK_DESTROY_LAMBDA_SIGNATURE(VkCommandPool)
	{
		vkDestroyCommandPool(vulkanInstance.GetLogicalDevice(), handle, nullptr);
	}),
		queueType(queueType),
		offsetPool(1, 16)
	{
		switch (queueType)
		{
		case EQueueType::Graphics:
			SetName("Graphics Queue");
			break;
		case EQueueType::Compute:
			SetName("Compute Queue");
			break;
		case EQueueType::Transfer:
			SetName("Transfer Queue");
			break;
		case EQueueType::Present:
			SetName("Present Queue");
			break;
		}

		const auto queueFamilyIdx = vulkanInstance.GetQueueFamilyIndex(queueType);
		const VkCommandPoolCreateInfo cmdPoolCreateInfo
		{
			.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
			.pNext = nullptr,
			.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
			.queueFamilyIndex = queueFamilyIdx
		};

		const size_t threadId = std::hash<std::thread::id>{}(std::this_thread::get_id());
		//spdlog::trace("Creating command pool for thread {} and queue family {}; Dependent on In-flight frames {}/{}.", threadId, queueFamilyIdx, (inFlightFrameIdx + 1), NumMaxInFlightFrames);
		VK_ASSERT(vkCreateCommandPool(vulkanInstance.GetLogicalDevice(), &cmdPoolCreateInfo, nullptr, &handle), "Failed to create vulkan command queue from create info.");
	}

	CommandPool::CommandBufferAllocation CommandPool::RequestCommandBuffer(std::string_view name)
	{
		const auto allocatedSlot = offsetPool.Allocate();
		const Deallocation deallocation
		{
			.slot = allocatedSlot
		};

		if (allocatedSlot.Offset >= cmdBuffers.size())
		{
			cmdBuffers.emplace_back(std::make_unique<CommandBuffer>(name, vulkanInstance, *this));
		}

		cmdBuffers[allocatedSlot.Offset]->Reset();
		return {
			cmdBuffers[allocatedSlot.Offset].get(),
			[this, deallocation](CommandBuffer*)
			{
				pendingDeallocations.emplace_back(deallocation);
			}
		};
	}

	void CommandPool::Reset() const
	{
		vkResetCommandPool(vulkanInstance.GetLogicalDevice(), handle, VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);
	}

	void CommandPool::BeginFrame()
	{
		for (auto& deallocation : pendingDeallocations)
		{
			offsetPool.Deallocate(deallocation.slot);
		}
		pendingDeallocations.clear();
		Reset();
	}
}