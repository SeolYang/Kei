#include <PCH.h>
#include <Render/RenderPasses/SimpleRenderPass.h>
#include <VK/CommandBuffer.h>
#include <VK/CommandPool.h>
#include <VK/CommandPoolManager.h>
#include <VK/VulkanContext.h>
#include <VK/Buffer.h>
#include <VK/DescriptorManager.h>
#include <VK/FrameTracker.h>
#include <VK/Swapchain.h>

#include "Render/Mesh.h"

namespace sy::render
{
	SimpleRenderPass::SimpleRenderPass(std::string_view name, const vk::VulkanContext& vulkanContext, vk::DescriptorManager& descriptorManager, const vk::FrameTracker& frameTracker, const vk::Pipeline& pipeline) :
	RenderPass(name, vulkanContext, descriptorManager, frameTracker, pipeline),
	vertexBuffer(VK_NULL_HANDLE),
	indexBuffer(VK_NULL_HANDLE),
	textureDescriptor(-1)
	{
		for (size_t idx = 0; idx < vk::NumMaxInFlightFrames; ++idx)
		{
			transformBuffers[idx] = vk::CreateUniformBuffer<TransformUniformBuffer>("TransformBuffer", vulkanContext);
			transformBufferIndices[idx] = descriptorManager.RequestDescriptor(*transformBuffers[idx]);
		}
	}

	vk::ManagedCommandBuffer SimpleRenderPass::Render(vk::CommandPoolManager& cmdPoolManager)
	{
		const auto& frameTracker = GetFrameTracker();
		const auto& descriptorManager = GetDescriptorManager();
		const auto& pipeline = GetPipeline();

		auto& graphicsCmdPool = cmdPoolManager.RequestCommandPool(vk::EQueueType::Graphics);
		auto graphicsCmdBuffer = graphicsCmdPool.RequestCommandBuffer("Render Cmd Buffer");
		graphicsCmdBuffer->Begin();
		{
			graphicsCmdBuffer->ChangeImageAccessPattern(vk::EAccessPattern::None, vk::EAccessPattern::ColorAttachmentWrite, swapchainImage, VK_IMAGE_ASPECT_COLOR_BIT);

			std::array colorAttachmentInfos = { swapchainAttachmentInfo };
			std::array depthAttachmentInfos = { depthAttachmentInfo };

			const VkRenderingInfo renderingInfo
			{
				.sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR,
				.pNext = nullptr,
				.renderArea = VkRect2D
				{
					.offset = VkOffset2D{0, 0},
					.extent = VkExtent2D{windowExtent.width, windowExtent.height},
				},
				.layerCount = 1,
				.colorAttachmentCount = static_cast<uint32_t>(colorAttachmentInfos.size()),
				.pColorAttachments = colorAttachmentInfos.data(),
				.pDepthAttachment = depthAttachmentInfos.data(),
				.pStencilAttachment = depthAttachmentInfos.data()
			};

			graphicsCmdBuffer->BeginRendering(renderingInfo);
			{
				// Rendering something here
				graphicsCmdBuffer->BindPipeline(pipeline);
				graphicsCmdBuffer->BindDescriptorSet(descriptorManager.GetDescriptorSet(), pipeline);

				const auto& transformBuffer = *transformBuffers[frameTracker.GetCurrentInFlightFrameIndex()];
				const PushConstants pushConstants
				{
					.textureIndex = static_cast<int>(textureDescriptor),
					.transformDataIndex = static_cast<int>(transformBufferIndices[frameTracker.GetCurrentInFlightFrameIndex()]->Offset)
				};

				std::array vertexBuffers = { vertexBuffer };
				std::array offsets = { uint64_t() };
				graphicsCmdBuffer->BindVertexBuffers(0, vertexBuffers, offsets);
				graphicsCmdBuffer->BindIndexBuffer(indexBuffer);
				graphicsCmdBuffer->PushConstants(pipeline, VK_SHADER_STAGE_ALL_GRAPHICS, pushConstants);

				graphicsCmdBuffer->DrawIndexed(numIndices, 1, 0, 0, 0);
			}
			graphicsCmdBuffer->EndRendering();
			graphicsCmdBuffer->ChangeImageAccessPattern(vk::EAccessPattern::ColorAttachmentWrite, vk::EAccessPattern::Present, swapchainImage, VK_IMAGE_ASPECT_COLOR_BIT);
		}
		graphicsCmdBuffer->End();

		return graphicsCmdBuffer;
	}

	void SimpleRenderPass::UpdateBuffers()
	{
		const auto& vulkanContext = GetVulkanContext();
		const auto& frameTracker = GetFrameTracker();
		const auto& transformBuffer = *transformBuffers[frameTracker.GetCurrentInFlightFrameIndex()];
		void* transformBufferMappedPtr = vulkanContext.Map(transformBuffer);
		memcpy(transformBufferMappedPtr, &transformData, sizeof(TransformUniformBuffer));
		vulkanContext.Unmap(transformBuffer);
	}

	void SimpleRenderPass::SetMesh(const Mesh& mesh)
	{
		const auto& vertexBuffer = mesh.GetVertexBuffer();
		const auto& indexBuffer = mesh.GetIndexBuffer();
		this->vertexBuffer = vertexBuffer.GetNativeHandle();
		this->indexBuffer = indexBuffer.GetNativeHandle();
		this->numVertices = mesh.GetNumVertices();
		this->numIndices = mesh.GetNumIndices();
	}

	void SimpleRenderPass::SetTextureDescriptor(const vk::Descriptor& descriptor)
	{
		this->textureDescriptor = descriptor->Offset;
	}

	void SimpleRenderPass::SetWindowExtent(Extent2D<uint32_t> extent)
	{
		this->windowExtent = extent;
	}

	void SimpleRenderPass::SetSwapchain(const vk::Swapchain& swapchain, VkClearColorValue clearColorValue)
	{
		swapchainImage = swapchain.GetCurrentImage();
		swapchainAttachmentInfo = swapchain.GetColorAttachmentInfo(clearColorValue);
	}

	void SimpleRenderPass::SetDepthStencilView(const vk::TextureView& depthStencilView)
	{
		depthAttachmentInfo = vk::DepthAttachmentInfo(depthStencilView);
	}

	void SimpleRenderPass::SetTransformData(const TransformUniformBuffer buffer)
	{
		transformData = buffer;
	}
}
