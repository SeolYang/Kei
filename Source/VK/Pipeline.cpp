#include <PCH.h>
#include <VK/Pipeline.h>
#include <VK/PipelineBuilder.h>
#include <VK/VulkanRHI.h>

namespace sy
{
	namespace vk
	{
		Pipeline::Pipeline(const std::string_view name, const VulkanRHI& vulkanRHI, const GraphicsPipelineBuilder& builder)
			: Pipeline(name, vulkanRHI, EPipelineType::Graphics, builder.GetLayout())
		{
			const auto createInfo = builder.Build();
			NativeHandle handle = VK_NULL_HANDLE;
			VK_ASSERT(vkCreateGraphicsPipelines(vulkanRHI.GetDevice(), VK_NULL_HANDLE, 1, &createInfo, nullptr, &handle),
				"Failed to create graphics pipeline {}.", name);

			UpdateHandle(
				handle, SY_VK_WRAPPER_DELETER(rhi) {
					vkDestroyPipeline(rhi.GetDevice(), handle, nullptr);
				});
		}

		Pipeline::Pipeline(const std::string_view name, const VulkanRHI& vulkanRHI, const ComputePipelineBuilder& builder)
			: Pipeline(name, vulkanRHI, EPipelineType::Compute, builder.GetLayout())
		{
			const auto createInfo = builder.Build();
			NativeHandle handle = VK_NULL_HANDLE;
			VK_ASSERT(vkCreateComputePipelines(vulkanRHI.GetDevice(), VK_NULL_HANDLE, 1, &createInfo, nullptr, &handle),
				"Failed to create compute pipeline {}.", name);

			UpdateHandle(
				handle, SY_VK_WRAPPER_DELETER(rhi) {
					vkDestroyPipeline(rhi.GetDevice(), handle, nullptr);
				});
		}

		Pipeline::Pipeline(const std::string_view name, const VulkanRHI& vulkanRHI, const EPipelineType pipelineType, VkPipelineLayout layout)
			: VulkanWrapper<VkPipeline>(name, vulkanRHI, VK_OBJECT_TYPE_PIPELINE)
			, pipelineType(pipelineType)
			, layout(layout)
		{
		}
	} // namespace vk
} // namespace sy
