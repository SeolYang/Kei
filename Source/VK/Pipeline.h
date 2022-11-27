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

	private:
		Pipeline(std::string_view name, const VulkanInstance& vulkanInstance, EPipelineType pipelineType);

	private:
		const EPipelineType pipelineType;

	};
}