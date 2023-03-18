#pragma once
#include <PCH.h>
#include <VK/VulkanWrapper.h>

namespace sy::vk
{
	class GraphicsPipelineBuilder;
	class ComputePipelineBuilder;
	class RayTracingPipelineBuilder;
	class VulkanContext;
	class Pipeline : public VulkanWrapper<VkPipeline>
	{
	public:
		Pipeline(std::string_view name, VulkanContext& vulkanContext, const GraphicsPipelineBuilder& builder);
		Pipeline(std::string_view name, VulkanContext& vulkanContext, const ComputePipelineBuilder& builder);
		~Pipeline() override = default;

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
		Pipeline(std::string_view name, VulkanContext& vulkanContext, EPipelineType pipelineType, VkPipelineLayout layout);

	private:
		const EPipelineType pipelineType;
		VkPipelineLayout layout;
	};
} // namespace sy::vk
