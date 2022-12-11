#pragma once
#include <Core/Core.h>

namespace sy
{
	namespace vk
	{
		class VulkanContext;
		class PipelineLayoutCache
		{
		public:
			struct PipelineLayoutInfo
			{
			public:
				bool operator==(const PipelineLayoutInfo& rhs) const;
				size_t hash() const noexcept;

			public:
				VkPipelineLayoutCreateFlags Flags;
				std::vector<VkDescriptorSetLayout> DescriptorSetLayouts;
				std::vector<VkPushConstantRange> PushConstantRanges;

			};

			struct PipelineLayoutHash
			{
			public:
				size_t operator()(const PipelineLayoutInfo& info) const noexcept
				{
					return info.hash();
				}
			};

		public:
			explicit PipelineLayoutCache(const VulkanContext& vulkanContext);
			PipelineLayoutCache(const PipelineLayoutCache&) = delete;
			PipelineLayoutCache(PipelineLayoutCache&&) = delete;

			PipelineLayoutCache& operator=(const PipelineLayoutCache&) = delete;
			PipelineLayoutCache& operator=(PipelineLayoutCache&&) = delete;

			~PipelineLayoutCache();

			VkPipelineLayout Request(const std::span<VkDescriptorSetLayout> descriptorSetLayouts, const std::span<VkPushConstantRange> pushConstantRanges, VkPipelineLayoutCreateFlags flags = 0);

		private:
			const VulkanContext& vulkanContext;
			robin_hood::unordered_map<PipelineLayoutInfo, VkPipelineLayout, PipelineLayoutHash> cache;

		};
	}
}
