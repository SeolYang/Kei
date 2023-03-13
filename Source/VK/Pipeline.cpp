#include <PCH.h>
#include <VK/Pipeline.h>
#include <VK/PipelineBuilder.h>
#include <VK/VulkanRHI.h>

namespace sy
{
	namespace vk
	{
		Pipeline::Pipeline(std::string_view name, const VulkanRHI& vulkanRHI,
		                   const GraphicsPipelineBuilder& builder) :
			Pipeline(name, vulkanRHI, EPipelineType::Graphics, builder.GetLayout())
		{
			const auto createInfo = builder.Build();
			Native_t handle       = VK_NULL_HANDLE;
			VK_ASSERT(vkCreateGraphicsPipelines(vulkanRHI.GetDevice(), VK_NULL_HANDLE, 1, &createInfo, nullptr, &handle
			          ), "Failed to create graphics pipeline {}.", name);
			UpdateHandle(handle);
		}

		Pipeline::Pipeline(std::string_view name, const VulkanRHI& vulkanRHI,
		                   const ComputePipelineBuilder& builder) :
			Pipeline(name, vulkanRHI, EPipelineType::Compute, builder.GetLayout())
		{
			const auto createInfo = builder.Build();
			Native_t handle       = VK_NULL_HANDLE;
			VK_ASSERT(vkCreateComputePipelines(vulkanRHI.GetDevice(), VK_NULL_HANDLE, 1, &createInfo, nullptr, &handle),
			          "Failed to create compute pipeline {}.", name);
			UpdateHandle(handle);
		}

		Pipeline::Pipeline(std::string_view name, const VulkanRHI& vulkanRHI, EPipelineType pipelineType,
		                   VkPipelineLayout layout) :
			VulkanWrapper<VkPipeline>(name, vulkanRHI, VK_OBJECT_TYPE_PIPELINE, VK_DESTROY_LAMBDA_SIGNATURE(VkPipeline)
			{
				vkDestroyPipeline(vulkanRHI.GetDevice(), handle, nullptr);
			}),
			pipelineType(pipelineType),
			layout(layout)
		{
		}
	}
}
