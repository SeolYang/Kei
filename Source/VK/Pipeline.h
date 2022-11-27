#pragma once
#include <Core.h>

namespace sy
{
	class GraphicsPipelineBuilder;
	class ComputePipelineBuilder;
	class RayTracingPipelineBuilder;
	class VulkanInstance;
	class Pipeline : public VulkanWrapper<VkPipeline>
	{
	public:
		Pipeline(std::string_view name, const VulkanInstance& vulkanInstance, const GraphicsPipelineBuilder & builder);
		Pipeline(std::string_view name, const VulkanInstance& vulkanInstance, const ComputePipelineBuilder& builder);

		[[nodiscard]] auto GetPipelineType() const { return pipelineType; }
		[[nodiscard]] auto GetPipelineBindPoint() const { return PipelineTypeToBindPoint(pipelineType); }
		[[nodiscard]] auto GetPipelineLayout() const { return layout; }

	private:
		Pipeline(std::string_view name, const VulkanInstance& vulkanInstance, EPipelineType pipelineType, VkPipelineLayout layout);

	private:
		const EPipelineType pipelineType;
		VkPipelineLayout layout;

	};
}