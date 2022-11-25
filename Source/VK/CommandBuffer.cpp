#include <Core.h>
#include <VK/CommandBuffer.h>
#include <VK/CommandPool.h>
#include <VK/VulkanInstance.h>

#include "Fence.h"

namespace sy
{
	CommandBuffer::CommandBuffer(std::string_view name, const VulkanInstance& vulkanInstance, const CommandPool& cmdPool) :
		VulkanWrapper<VkCommandBuffer>(name, vulkanInstance, VK_DESTROY_LAMBDA_SIGNATURE(VkCommandBuffer)
		{
			// DO NOTHING
		}),
		queueType(cmdPool.GetQueueType()),
		cmdPool(cmdPool),
		dependencyFence(nullptr)
	{
		const VkCommandBufferAllocateInfo allocInfo
		{
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			.pNext = nullptr,
			.commandPool = cmdPool.GetNativeHandle(),
			.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			.commandBufferCount = 1
		};

		VK_ASSERT(vkAllocateCommandBuffers(vulkanInstance.GetLogicalDevice(), &allocInfo, &handle), "Failed to creating command buffer.");
	}

	bool CommandBuffer::IsReadyToUse() const
	{
		return dependencyFence == nullptr || dependencyFence->IsSignaled();
	}

	void CommandBuffer::Begin(const Fence& newDependencyFence)
	{
		dependencyFence = &newDependencyFence;
		const VkCommandBufferBeginInfo beginInfo
		{
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			.pNext = nullptr,
			.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
			.pInheritanceInfo =  nullptr
		};

		vkResetCommandBuffer(handle, 0);
		VK_ASSERT(vkBeginCommandBuffer(handle, &beginInfo), "Faeild to begin command buffer {}.", GetName());
	}

	void CommandBuffer::End() const
	{
		VK_ASSERT(vkEndCommandBuffer(handle), "Failed to end command buffer {}.", GetName());
	}
}
