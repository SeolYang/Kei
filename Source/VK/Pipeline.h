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
		Pipeline(const GraphicsPipelineBuilder& builder);

	private:
		const VulkanInstance& vulkanInstance;
		const EPipelineType pipelineType;

	};
}