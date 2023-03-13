#pragma once
#include <PCH.h>

namespace sy::vk
{
	class VulkanRHI;

	class Fence : public VulkanWrapper<VkFence>
	{
	public:
		Fence(std::string_view name, const VulkanRHI& vulkanRHI, bool bIsSignaled = true);
		virtual ~Fence() override = default;

		void Wait() const;
		void Reset() const;
		[[nodiscard]] bool IsSignaled() const;
	};
}
