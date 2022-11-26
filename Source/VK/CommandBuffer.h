#pragma once
#include <Core.h>

namespace sy
{
	class VulkanInstance;
	class CommandPool;
	class Fence;
	class CommandBuffer : public VulkanWrapper<VkCommandBuffer>
	{
	public:
		CommandBuffer(std::string_view name, const VulkanInstance& vulkanInstance, const CommandPool& cmdPool);
		virtual ~CommandBuffer() override = default;

		[[nodiscard]] EQueueType GetQueueType() const { return queueType; }

		void Reset() const;
		void Begin();
		void End() const;

		void BeginRendering(const VkRenderingInfo& renderingInfo) const;
		void EndRendering() const;

	private:
		const EQueueType queueType;

	};

}
