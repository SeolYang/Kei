#include <PCH.h>
#include <VK/LayoutCache.h>
#include <VK/VulkanContext.h>
#include <VK/VulkanRHI.h>
#include <VK/PushConstantBuilder.h>

namespace sy::vk
{
	PipelineLayoutCache::PipelineLayoutCache(VulkanContext& vulkanContext)
		: vulkanContext(vulkanContext)
	{
	}

	PipelineLayoutCache::~PipelineLayoutCache()
	{
	}

	void PipelineLayoutCache::Startup()
	{
		spdlog::info("Startup Pipeline Layout Cache.");
	}

	void PipelineLayoutCache::Shutdown()
	{
		spdlog::info("Shutdown Pipeline Layout Cache.");
		const auto device = vulkanContext.GetRHI().GetDevice();
		for (auto& cacheInfo : cache)
		{
			vkDestroyPipelineLayout(device, cacheInfo.second, nullptr);
		}
	}

	bool PipelineLayoutCache::PipelineLayoutInfo::operator==(const PipelineLayoutInfo& rhs) const
	{
		const bool bHasSameNumOfDescriptorSetLayouts = DescriptorSetLayouts.size() == rhs.DescriptorSetLayouts.size();
		const bool bHasSameNumOfPushConstantRanges = PushConstantRanges.size() == rhs.PushConstantRanges.size();
		const bool bHasSameFlags = Flags == rhs.Flags;
		if (!(bHasSameNumOfDescriptorSetLayouts && bHasSameNumOfPushConstantRanges && bHasSameFlags))
		{
			return false;
		}

		for (size_t idx = 0; idx < DescriptorSetLayouts.size(); ++idx)
		{
			if (DescriptorSetLayouts[idx] != rhs.DescriptorSetLayouts[idx])
			{
				return false;
			}
		}

		for (size_t idx = 0; idx < PushConstantRanges.size(); ++idx)
		{
			const auto& pushConstantRange = PushConstantRanges[idx];
			const auto& rhsPushConstantRange = rhs.PushConstantRanges[idx];
			const bool bHasSameConstantRangeSize = pushConstantRange.size == rhsPushConstantRange.size;
			const bool bHasSameConstantRangeOffset = pushConstantRange.offset == rhsPushConstantRange.offset;
			const bool bHasSameConstantRangeStageFlags = pushConstantRange.stageFlags == rhsPushConstantRange.stageFlags;
			if (!(bHasSameConstantRangeSize && bHasSameConstantRangeOffset && bHasSameConstantRangeStageFlags))
			{
				return false;
			}
		}

		return true;
	}

	size_t PipelineLayoutCache::PipelineLayoutInfo::hash() const noexcept
	{
		size_t result = (DescriptorSetLayouts.size() ^ PushConstantRanges.size() ^ Flags);
		for (const auto descriptorSetLayout : DescriptorSetLayouts)
		{
			result ^= std::hash<size_t>()(reinterpret_cast<size_t>(descriptorSetLayout));
		}

		for (const auto& pushConstantRange : PushConstantRanges)
		{
			const size_t constantRangeHash = std::hash<size_t>()(pushConstantRange.size | pushConstantRange.offset << 8 | pushConstantRange.stageFlags << 16);
			result ^= constantRangeHash;
		}

		return result;
	}

	VkPipelineLayout PipelineLayoutCache::Request(const std::span<VkDescriptorSetLayout> descriptorSetLayouts,
		const PushConstantBuilder& pushConstantBuilder)
	{
		PipelineLayoutInfo pipelineLayoutInfo{
			.Flags = 0,
		};

		const auto pushConstantRanges = pushConstantBuilder.Build();
		pipelineLayoutInfo.DescriptorSetLayouts.assign(descriptorSetLayouts.begin(), descriptorSetLayouts.end());
		pipelineLayoutInfo.PushConstantRanges.assign(pushConstantRanges.begin(), pushConstantRanges.end());

		if (!cache.contains(pipelineLayoutInfo))
		{
			const VkPipelineLayoutCreateInfo createInfo{
				.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
				.pNext = nullptr,
				.flags = 0,
				.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size()),
				.pSetLayouts = descriptorSetLayouts.data(),
				.pushConstantRangeCount = static_cast<uint32_t>(pushConstantRanges.size()),
				.pPushConstantRanges = pushConstantRanges.data()
			};

			const auto& vulkanRHI = vulkanContext.GetRHI();
			VkPipelineLayout newPipelineLayout = VK_NULL_HANDLE;
			VK_ASSERT(vkCreatePipelineLayout(vulkanRHI.GetDevice(), &createInfo, nullptr, &newPipelineLayout),
				"Failed to create new pipeline layout.");
			cache[pipelineLayoutInfo] = newPipelineLayout;
		}

		return cache[pipelineLayoutInfo];
	}

} // namespace sy::vk
