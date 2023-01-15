#pragma once
#include <PCH.h>
#include <Render/RenderPass.h>

namespace sy::vk
{
	class Swapchain;
}

namespace sy::render
{
	struct TransformUniformBuffer
	{
		glm::mat4 modelViewProj;
	};

	struct PushConstants
	{
		int textureIndex;
		int transformDataIndex;
	};

	class Mesh;
	class SimpleRenderPass final : public RenderPass
	{
	public:
		SimpleRenderPass(std::string_view name, const vk::VulkanContext& vulkanContext, vk::DescriptorManager& descriptorManager, const vk::FrameTracker& frameTracker, vk::CommandPoolManager& cmdPoolManager, const vk::Pipeline& pipeline);

		virtual void OnBegin() override;
		virtual void Render() override;
		virtual void OnEnd() override;
		virtual void UpdateBuffers() override;

		void SetMesh(const Mesh& mesh);
		void SetTextureDescriptor(const vk::Descriptor& descriptor);
		void SetWindowExtent(Extent2D<uint32_t> extent);
		void SetSwapchain(const vk::Swapchain& swapchain, VkClearColorValue clearColorValue);
		void SetDepthStencilView(const vk::TextureView& depthStencilView);
		void SetTransformData(TransformUniformBuffer buffer);

	private:
		VkBuffer vertexBuffer;
		VkBuffer indexBuffer;
		size_t numVertices;
		size_t numIndices;
		size_t textureDescriptor;

		std::array<std::unique_ptr<vk::Buffer>, vk::NumMaxInFlightFrames> transformBuffers;
		std::array<vk::Descriptor, vk::NumMaxInFlightFrames> transformBufferIndices;

		Extent2D<uint32_t> windowExtent;
		VkImage swapchainImage;
		VkRenderingAttachmentInfo swapchainAttachmentInfo;
		VkRenderingAttachmentInfo depthAttachmentInfo;

		TransformUniformBuffer transformData;

	};
}
