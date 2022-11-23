#include <Core.h>
#include <CommandPool.h>
#include <CommandBuffer.h>
#include <VulkanInstance.h>

namespace sy
{
	CommandPool::CommandPool(const VulkanInstance& vulkanInstance, const EQueueType queueType) :
		NamedType("Unknown Queue"),
		vulkanInstance(vulkanInstance),
		queueType(queueType),
		pool(VK_NULL_HANDLE)
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

		const VkCommandPoolCreateInfo cmdPoolCreateInfo
		{
			.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
			.pNext = nullptr,
			.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
			.queueFamilyIndex = vulkanInstance.GetQueueFamilyIndex(queueType)
		};

		const size_t threadId = std::hash<std::thread::id>{}(std::this_thread::get_id());
		spdlog::trace("Creating cmd pool for thread {}.", threadId);
		VK_ASSERT(vkCreateCommandPool(vulkanInstance.GetLogicalDevice(), &cmdPoolCreateInfo, nullptr, &pool), "Failed to create vulkan command queue from create info.");
	}

	CommandPool::~CommandPool()
	{
		vkDestroyCommandPool(vulkanInstance.GetLogicalDevice(), pool, nullptr);
	}

	const CommandBuffer& CommandPool::RequestCommandBuffer(const std::string_view name)
	{
		spdlog::trace("Request Command Buffer {} from Command Pool {}.", name, this->GetName());
		for (const auto& cmdBufferPtr : cmdBuffers)
		{
			if (cmdBufferPtr->IsReadyToUse())
			{
				cmdBufferPtr->ResetFence();
				cmdBufferPtr->SetName(name);
				return *cmdBufferPtr;
			}
		}

		cmdBuffers.emplace_back(std::make_unique<CommandBuffer>(name, vulkanInstance, *this));
		return *cmdBuffers.back();
	}
}
