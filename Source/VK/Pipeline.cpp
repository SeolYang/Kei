#include <PCH.h>
#include <VK/Pipeline.h>
#include <VK/PipelineBuilder.h>
#include <VK/VulkanContext.h>

namespace sy
{
	namespace vk
	{
		Pipeline::Pipeline(std::string_view name, const VulkanContext& vulkanContext,
			const GraphicsPipelineBuilder& builder) :
			Pipeline(name, vulkanContext, EPipelineType::Graphics, builder.GetLayout())
		{
			const auto createInfo = builder.Build();
			Native_t handle = VK_NULL_HANDLE;
			VK_ASSERT(vkCreateGraphicsPipelines(vulkanContext.GetDevice(), VK_NULL_HANDLE, 1, &createInfo, nullptr, &handle), "Failed to create graphics pipeline {}.", name);
			UpdateHandle(handle);
		}

		Pipeline::Pipeline(std::string_view name, const VulkanContext& vulkanContext,
			const ComputePipelineBuilder& builder) :
			Pipeline(name, vulkanContext, EPipelineType::Compute, builder.GetLayout())
		{
			const auto createInfo = builder.Build();
			Native_t handle = VK_NULL_HANDLE;
			VK_ASSERT(vkCreateComputePipelines(vulkanContext.GetDevice(), VK_NULL_HANDLE, 1, &createInfo, nullptr, &handle), "Failed to create compute pipeline {}.", name);
			UpdateHandle(handle);
		}

		Pipeline::Pipeline(std::string_view name, const VulkanContext& vulkanContext, EPipelineType pipelineType, VkPipelineLayout layout) :
			VulkanWrapper<VkPipeline>(name, vulkanContext, VK_OBJECT_TYPE_PIPELINE, VK_DESTROY_LAMBDA_SIGNATURE(VkPipeline)
		{
			vkDestroyPipeline(vulkanContext.GetDevice(), handle, nullptr);
		}),
			pipelineType(pipelineType),
			layout(layout)
		{
		}
	}
}