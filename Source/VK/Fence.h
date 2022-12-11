#pragma once
#include <Core/Core.h>

namespace sy
{
	namespace vk
	{
		class VulkanContext;
		class Fence : public VulkanWrapper<VkFence>
		{
		public:
			Fence(std::string_view name, const VulkanContext& vulkanContext, bool bIsSignaled = true);
			virtual ~Fence() override = default;

			void Wait() const;
			void Reset() const;
			[[nodiscard]] bool IsSignaled() const;

		};
	}
}
