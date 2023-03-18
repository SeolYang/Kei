#pragma once
#include <PCH.h>
#include <VK/VulkanWrapper.h>

namespace sy::vk
{
	class VulkanContext;
	class Fence : public VulkanWrapper<VkFence>
	{
	public:
		Fence(std::string_view name, VulkanContext& vulkanContext, bool bIsSignaled = true);
		~Fence() override = default;

		void Wait() const;
		void Reset() const;
		[[nodiscard]] bool IsSignaled() const;
	};
} // namespace sy::vk
