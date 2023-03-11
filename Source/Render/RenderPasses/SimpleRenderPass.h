#pragma once
#include <PCH.h>
#include <Render/RenderPass.h>

namespace sy
{
	class ResourceCache;
}

namespace sy::vk
{
	class Swapchain;
	class Sampler;
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
	class SimpleRenderPass : public RenderPass
	{
	public:
		SimpleRenderPass(std::string_view name, ResourceCache& resourceCache, const vk::VulkanContext& vulkanContext, const vk::Pipeline& pipeline);

		virtual void OnBegin() override;
		virtual void Render() override;
		virtual void OnEnd() override;
		virtual void UpdateBuffers() override;

		void SetMesh(Handle<Mesh> mesh);
		void SetTextureDescriptor(Handle<vk::Descriptor> descriptor);
		void SetWindowExtent(Extent2D<uint32_t> extent);
		void SetSwapchain(const vk::Swapchain& swapchain, VkClearColorValue clearColorValue);
		void SetDepthStencilView(const vk::TextureView& depthStencilView);
		void SetTransformData(TransformUniformBuffer buffer);

	private:
		ResourceCache& resourceCache;
		Handle<Mesh> mesh;
		Handle<vk::Descriptor> descriptor;

		std::array<std::unique_ptr<vk::Buffer>, vk::NumMaxInFlightFrames> transformBuffers;
		std::array<vk::Descriptor, vk::NumMaxInFlightFrames> transformBufferIndices;

		Extent2D<uint32_t> windowExtent;
		VkImage swapchainImage;
		VkRenderingAttachmentInfo swapchainAttachmentInfo;
		VkRenderingAttachmentInfo depthAttachmentInfo;

		TransformUniformBuffer transformData;

	};
}
