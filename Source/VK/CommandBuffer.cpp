#include <Core.h>
#include <VK/CommandBuffer.h>
#include <VK/VulkanInstance.h>
#include <VK/CommandPool.h>

#include "Fence.h"

namespace sy
{
	CommandBuffer::CommandBuffer(std::string_view name, const VulkanInstance& vulkanInstance, const CommandPool& cmdPool) :
		NamedType(name),
		queueType(cmdPool.GetQueueType()),
		vulkanInstance(vulkanInstance),
		cmdPool(cmdPool),
		fence(std::make_unique<Fence>(name, vulkanInstance))
	{
		const VkCommandBufferAllocateInfo allocInfo
		{
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			.pNext = nullptr,
			.commandPool = cmdPool.GetCommandPool(),
			.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			.commandBufferCount = 1
		};

		const size_t threadId = std::hash<std::thread::id>{}(std::this_thread::get_id());
		spdlog::trace("Creating cmd buffer for thread {}.", threadId);
		VK_ASSERT(vkAllocateCommandBuffers(vulkanInstance.GetLogicalDevice(), &allocInfo, &cmdBuffer), "Failed to creating command buffer.");
	}

	bool CommandBuffer::IsReadyToUse() const
	{
		return fence->IsSignaled();
	}

	const CommandBuffer& CommandBuffer::ResetFence() const
	{
		fence->Reset();
		return *this;
	}

	const CommandBuffer& CommandBuffer::Begin() const
	{
		const VkCommandBufferBeginInfo beginInfo
		{
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			.pNext = nullptr,
			.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
			.pInheritanceInfo =  nullptr
		};

		VK_ASSERT(vkBeginCommandBuffer(cmdBuffer, &beginInfo), "Faeild to begin command buffer {}.", GetName());
		return *this;
	}

	const CommandBuffer& CommandBuffer::End() const
	{
		VK_ASSERT(vkEndCommandBuffer(cmdBuffer), "Failed to end command buffer {}.", GetName());
		return *this;
	}
}
