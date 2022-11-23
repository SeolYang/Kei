#pragma once
#include <Core.h>

namespace sy
{
	class VulkanInstance;
	class Fence : public NamedType
	{
	public:
		Fence(std::string_view name, const VulkanInstance& vulkanInstance, bool bIsSignaled = true);
		virtual ~Fence() override;

		[[nodiscard]] VkFence GetFence() const { return fence; }

		const Fence& Wait() const;
		const Fence& Reset() const;
		[[nodiscard]] bool IsSignaled() const;

	private:
		const VulkanInstance& vulkanInstance;
		VkFence fence;

	};
}
