#include <Core.h>
#include <Renderer.h>
#include <Window.h>
#include <VK/VulkanContext.h>
#include <VK/Fence.h>
#include <VK/Semaphore.h>
#include <VK/Swapchain.h>
#include <VK/CommandPool.h>
#include <VK/CommandBuffer.h>
#include <VK/ShaderModule.h>
#include <VK/Pipeline.h>
#include <VK/PipelineBuilder.h>
#include <VK/LayoutCache.h>
#include <VK/Texture.h>
#include <VK/Buffer.h>
#include <DescriptorManager.h>
#include <CommandPoolManager.h>
#include <FrameTracker.h>

namespace sy
{
	struct ColorData
	{
		glm::vec4 colors[3];
	};

	struct PushConstants
	{
		int colorDataIndex;
		int textureIndex;
	};

	Renderer::Renderer(const Window& window, VulkanContext& vulkanContext, const FrameTracker& frameTracker, CommandPoolManager& cmdPoolManager, DescriptorManager& descriptorManager) :
		window(window),
		vulkanContext(vulkanContext),
		frameTracker(frameTracker),
		cmdPoolManager(cmdPoolManager),
		descriptorManager(descriptorManager),
		pipelineLayoutCache(std::make_unique<PipelineLayoutCache>(vulkanContext))
	{
		const auto windowExtent = window.GetExtent();

		triVert = std::make_unique<ShaderModule>("Triangle vertex shader", vulkanContext, "Assets/Shaders/bin/textured_tri_bindless.vert.spv", VK_SHADER_STAGE_VERTEX_BIT, "main");
		triFrag = std::make_unique<ShaderModule>("Triangle fragment shader", vulkanContext, "Assets/Shaders/bin/textured_tri_bindless.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT, "main");

		std::array descriptorSetLayouts = { descriptorManager.GetDescriptorSetLayout(), };
		std::array pushConstantRanges = {
			VkPushConstantRange{ VK_SHADER_STAGE_ALL_GRAPHICS, 0, sizeof(PushConstants) },
		};

		GraphicsPipelineBuilder basicPipelineBuilder;
		basicPipelineBuilder.SetDefault()
			.AddShaderStage(*triVert)
			.AddShaderStage(*triFrag)
			.AddViewport(0.f, 0.f, static_cast<float>(windowExtent.width), static_cast<float>(windowExtent.height), 0.0f, 1.0f)
			.AddScissor(0, 0, windowExtent.width, windowExtent.height)
			.SetPipelineLayout(pipelineLayoutCache->Request(descriptorSetLayouts, pushConstantRanges));
		basicPipeline = std::make_unique<Pipeline>("Basic Graphics Pipeline", vulkanContext, basicPipelineBuilder);

		for (size_t idx = 0; idx < NumMaxInFlightFrames; ++idx)
		{
			colorBuffers[idx] = Buffer::CreateUniformBuffer<ColorData>("ColorBuffer", vulkanContext);
			descriptorIndices[idx] = descriptorManager.RequestDescriptor(*colorBuffers[idx]);
		}

		loadedTexture = Texture2D::LoadFromFile(cmdPoolManager, frameTracker, "Assets/Textures/djmax_1st_anv.png", vulkanContext, VK_FORMAT_R8G8B8A8_SRGB);
		loadedTextureDescriptor = descriptorManager.RequestDescriptor(*loadedTexture);
	}

	Renderer::~Renderer()
	{
		vulkanContext.WaitAllQueuesForIdle();
	}

	void Renderer::Render()
	{
		BeginFrame();
		{
			auto& renderFence = frameTracker.GetCurrentInFlightRenderFence();
			auto& renderSemaphore = frameTracker.GetCurrentInFlightRenderSemaphore();
			auto& presentSemaphore = frameTracker.GetCurrentInFlightPresentSemaphore();
			const size_t currentFrameIdx = frameTracker.GetCurrentFrameIndex();

			const auto windowExtent = window.GetExtent();
			auto& swapchain = vulkanContext.GetSwapchain();
			const auto swapchainImage = swapchain.GetCurrentImage();
			const auto swapchainImageView = swapchain.GetCurrentImageView();

			auto& graphicsCmdPool = cmdPoolManager.RequestCommandPool(EQueueType::Graphics);
			CRefVec<CommandBuffer> graphicsCmdBufferBatch;
			const auto graphicsCmdBuffer = graphicsCmdPool.RequestCommandBuffer("Render Cmd Buffer");
			graphicsCmdBufferBatch.emplace_back(*graphicsCmdBuffer);
			graphicsCmdBuffer->Begin();
			{
				graphicsCmdBuffer->ChangeImageLayout(
					VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
					swapchainImage, VK_IMAGE_LAYOUT_UNDEFINED,
					VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
					VK_IMAGE_ASPECT_COLOR_BIT);

				VkClearColorValue clearColorValue;
				clearColorValue.float32[0] = std::cos(currentFrameIdx / 180.f) * 0.5f + 1.f;
				clearColorValue.float32[1] = std::sin(currentFrameIdx / 270.f) * 0.5f + 1.f;
				clearColorValue.float32[2] = std::cos(currentFrameIdx / 90.f) * 0.5f + 1.f;
				clearColorValue.float32[3] = 1.f;

				const auto colorAttachmentInfo = swapchain.GetColorAttachmentInfo(clearColorValue);
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
					.colorAttachmentCount = 1,
					.pColorAttachments = &colorAttachmentInfo
				};

				graphicsCmdBuffer->BeginRendering(renderingInfo);
				{
					// Rendering something here
					graphicsCmdBuffer->BindPipeline(*basicPipeline);
					graphicsCmdBuffer->BindDescriptorSet(descriptorManager.GetDescriptorSet(), *basicPipeline);

					ColorData colorData;
					colorData.colors[0] = { 1.f - clearColorValue.float32[0], 1.f - clearColorValue.float32[1], 1.f - clearColorValue.float32[2], 1.f };
					colorData.colors[1] = { colorData.colors[0].g, colorData.colors[0].r, colorData.colors[0].b, 1.f };
					colorData.colors[2] = { colorData.colors[0].b, colorData.colors[0].g, colorData.colors[0].r, 1.f };

					const auto& colorBuffer = *colorBuffers[frameTracker.GetCurrentInFlightFrameIndex()];
					void* colorBufferMappedPtr = vulkanContext.Map(colorBuffer);
					memcpy(colorBufferMappedPtr, &colorData, sizeof(ColorData));
					vulkanContext.Unmap(colorBuffer);

					const PushConstants pushConstants
					{
						.colorDataIndex = static_cast<int>(descriptorIndices[frameTracker.GetCurrentInFlightFrameIndex()]->Offset),
						.textureIndex = static_cast<int>(loadedTextureDescriptor->Offset)
					};

					graphicsCmdBuffer->PushConstants(*basicPipeline, VK_SHADER_STAGE_ALL_GRAPHICS, 0, sizeof(PushConstants), &pushConstants);

					graphicsCmdBuffer->Draw(3, 1, 0, 0);
				}
				graphicsCmdBuffer->EndRendering();

				graphicsCmdBuffer->ChangeImageLayout(
					VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
					swapchainImage, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
					VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
					VK_IMAGE_ASPECT_COLOR_BIT);
			}
			graphicsCmdBuffer->End();

			CRefVec<Semaphore> waitSemaphores;
			waitSemaphores.emplace_back(presentSemaphore);
			CRefVec<Semaphore> signalSemaphores;
			signalSemaphores.emplace_back(renderSemaphore);

			vulkanContext.SubmitTo(EQueueType::Graphics,
				waitSemaphores,
				graphicsCmdBufferBatch,
				signalSemaphores,
				VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, renderFence);

			vulkanContext.Present(swapchain, renderSemaphore);
		}
		EndFrame();
	}

	void Renderer::BeginFrame()
	{
		auto& swapchain = vulkanContext.GetSwapchain();
		swapchain.AcquireNext(frameTracker.GetCurrentInFlightPresentSemaphore());
		frameTracker.WaitForInFlightRenderFence();
		frameTracker.ResetInFlightRenderFence();
	}

	void Renderer::EndFrame()
	{
		/* Empty */
	}
}
