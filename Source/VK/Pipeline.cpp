#include <Core.h>
#include <VK/Pipeline.h>
#include <VK/PipelineBuilder.h>
#include <VK/VulkanInstance.h>

namespace sy
{
	Pipeline::Pipeline(std::string_view name, const VulkanInstance& vulkanInstance,
		const GraphicsPipelineBuilder& builder) :
		Pipeline(name, vulkanInstance, EPipelineType::Graphics)
	{
		const auto createInfo = builder.Build();
		VK_ASSERT(vkCreateGraphicsPipelines(vulkanInstance.GetLogicalDevice(), VK_NULL_HANDLE, 1, &createInfo, nullptr, &handle), "Failed to create graphics pipeline {}.", name);
	}

	Pipeline::Pipeline(std::string_view name, const VulkanInstance& vulkanInstance,
		const ComputePipelineBuilder& builder) :
		Pipeline(name, vulkanInstance, EPipelineType::Compute)
	{
		const auto createInfo = builder.Build();
		VK_ASSERT(vkCreateComputePipelines(vulkanInstance.GetLogicalDevice(), VK_NULL_HANDLE, 1, &createInfo, nullptr, &handle), "Failed to create compute pipeline {}.", name);
	}

	Pipeline::Pipeline(std::string_view name, const VulkanInstance& vulkanInstance, EPipelineType pipelineType) :
		VulkanWrapper<VkPipeline>(name, vulkanInstance, VK_DESTROY_LAMBDA_SIGNATURE(VkPipeline)
		{
			vkDestroyPipeline(vulkanInstance.GetLogicalDevice(), handle, nullptr);
		}),
		pipelineType(pipelineType)
	{
	}
}
