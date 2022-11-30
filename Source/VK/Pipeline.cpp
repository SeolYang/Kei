#include <Core.h>
#include <VK/Pipeline.h>
#include <VK/PipelineBuilder.h>
#include <VK/VulkanContext.h>

namespace sy
{
	Pipeline::Pipeline(std::string_view name, const VulkanContext& vulkanContext,
		const GraphicsPipelineBuilder& builder) :
		Pipeline(name, vulkanContext, EPipelineType::Graphics, builder.GetPipelineLayout())
	{
		const auto createInfo = builder.Build();
		VK_ASSERT(vkCreateGraphicsPipelines(vulkanContext.GetDevice(), VK_NULL_HANDLE, 1, &createInfo, nullptr, &handle), "Failed to create graphics pipeline {}.", name);
	}

	Pipeline::Pipeline(std::string_view name, const VulkanContext& vulkanContext,
		const ComputePipelineBuilder& builder) :
		Pipeline(name, vulkanContext, EPipelineType::Compute, builder.GetPipelineLayout())
	{
		const auto createInfo = builder.Build();
		VK_ASSERT(vkCreateComputePipelines(vulkanContext.GetDevice(), VK_NULL_HANDLE, 1, &createInfo, nullptr, &handle), "Failed to create compute pipeline {}.", name);
	}

	Pipeline::Pipeline(std::string_view name, const VulkanContext& vulkanContext, EPipelineType pipelineType, VkPipelineLayout layout) :
		VulkanWrapper<VkPipeline>(name, vulkanContext, VK_DESTROY_LAMBDA_SIGNATURE(VkPipeline)
		{
			vkDestroyPipeline(vulkanContext.GetDevice(), handle, nullptr);
		}),
		pipelineType(pipelineType),
		layout(layout)
	{
	}
}
