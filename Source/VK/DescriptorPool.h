#pragma once
#include <Core.h>

namespace sy
{
	class DescriptorPool : public VulkanWrapper<VkDescriptorPool>
	{
	public:
		DescriptorPool(std::string_view name, const VulkanInstance& vulkanInstance);
		virtual ~DescriptorPool() override;

		[[nodiscard]] auto GetDescriptorSetLayout() const { return descriptorSetLayout; }
		[[nodiscard]] auto GetDescriptorSet() const { return descriptorSet; }

		[[nodiscard]] UniqueHandle<OffsetPool::Slot_t> RequestUniformBufferDescriptorIndex();

		void EndFrame();

	private:
		VkDescriptorSetLayout descriptorSetLayout;
		VkDescriptorSet descriptorSet;

		OffsetPool uniformBufferPool;
		std::vector<OffsetPool::Slot_t> pendingUniformBufferDeallocations;

	};
}