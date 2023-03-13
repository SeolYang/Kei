#pragma once
#include <PCH.h>

namespace sy::vk
{
	class GraphicsPipelineBuilder;
	class ComputePipelineBuilder;
	class RayTracingPipelineBuilder;
	class VulkanRHI;

	class Pipeline : public VulkanWrapper<VkPipeline>
	{
	public:
		Pipeline(std::string_view name, const VulkanRHI& vulkanRHI, const GraphicsPipelineBuilder& builder);
		Pipeline(std::string_view name, const VulkanRHI& vulkanRHI, const ComputePipelineBuilder& builder);

		[[nodiscard]] auto GetPipelineType() const
		{
			return pipelineType;
		}

		[[nodiscard]] auto GetBindPoint() const
		{
			return ToNative(pipelineType);
		}

		[[nodiscard]] auto GetLayout() const
		{
			return layout;
		}

	private:
		Pipeline(std::string_view name, const VulkanRHI& vulkanRHI, EPipelineType pipelineType,
		         VkPipelineLayout layout);

	private:
		const EPipelineType pipelineType;
		VkPipelineLayout layout;
	};
}
