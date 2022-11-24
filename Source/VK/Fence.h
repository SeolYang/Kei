#pragma once
#include <Core.h>

namespace sy
{
	class VulkanInstance;
	class Fence : public VulkanWrapper<VkFence>
	{
	public:
		Fence(std::string_view name, const VulkanInstance& vulkanInstance, bool bIsSignaled = true);
		virtual ~Fence() override = default;

		void Wait() const;
		void Reset() const;
		[[nodiscard]] bool IsSignaled() const;

	};
}
