#include <PCH.h>
#include <VK/Pipeline.h>
#include <VK/PipelineBuilder.h>
#include <VK/VulkanContext.h>
#include <VK/VulkanRHI.h>

namespace sy
{
	namespace vk
	{
		Pipeline::Pipeline(const std::string_view name, VulkanContext& vulkanContext, const GraphicsPipelineBuilder& builder)
			: Pipeline(name, vulkanContext, EPipelineType::Graphics, builder.GetLayout())
		{
			const auto& vulkanRHI = vulkanContext.GetRHI();
			const auto createInfo = builder.Build();
			NativeHandle handle = VK_NULL_HANDLE;
			VK_ASSERT(vkCreateGraphicsPipelines(vulkanRHI.GetDevice(), VK_NULL_HANDLE, 1, &createInfo, nullptr, &handle),
				"Failed to create graphics pipeline {}.", name);

			UpdateHandle(
				handle, [handle](const VulkanRHI& rhi) {
					vkDestroyPipeline(rhi.GetDevice(), handle, nullptr);
				});
		}

		Pipeline::Pipeline(const std::string_view name, VulkanContext& vulkanContext, const ComputePipelineBuilder& builder)
			: Pipeline(name, vulkanContext, EPipelineType::Compute, builder.GetLayout())
		{
			const auto& vulkanRHI = vulkanContext.GetRHI();
			const auto createInfo = builder.Build();
			NativeHandle handle = VK_NULL_HANDLE;
			VK_ASSERT(vkCreateComputePipelines(vulkanRHI.GetDevice(), VK_NULL_HANDLE, 1, &createInfo, nullptr, &handle),
				"Failed to create compute pipeline {}.", name);

			UpdateHandle(
				handle, [handle](const VulkanRHI& rhi) {
					vkDestroyPipeline(rhi.GetDevice(), handle, nullptr);
				});
		}

		Pipeline::Pipeline(const std::string_view name, VulkanContext& vulkanContext, const EPipelineType pipelineType, VkPipelineLayout layout)
			: VulkanWrapper<VkPipeline>(name, vulkanContext, VK_OBJECT_TYPE_PIPELINE)
			, pipelineType(pipelineType)
			, layout(layout)
		{
		}
	} // namespace vk
} // namespace sy
