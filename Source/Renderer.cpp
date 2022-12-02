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
#include <DescriptorManager.h>
#include <CommandPoolManager.h>
#include <FrameTracker.h>

namespace sy
{
	Renderer::Renderer(const Window& window, VulkanContext& vulkanContext, const FrameTracker& frameTracker, CommandPoolManager& cmdPoolManager, DescriptorManager& descriptorManager) :
		window(window),
		vulkanContext(vulkanContext),
		frameTracker(frameTracker),
		cmdPoolManager(cmdPoolManager),
		descriptorManager(descriptorManager),
		pipelineLayoutCache(std::make_unique<PipelineLayoutCache>(vulkanContext))
	{
		const auto windowExtent = window.GetExtent();

		triVert = std::make_unique<ShaderModule>("Triangle vertex shader", vulkanContext, "Assets/Shaders/bin/tri.vert.spv", VK_SHADER_STAGE_VERTEX_BIT, "main");
		triFrag = std::make_unique<ShaderModule>("Triangle fragment shader", vulkanContext, "Assets/Shaders/bin/tri.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT, "main");

		std::array descriptorSetLayouts = { descriptorManager.GetDescriptorSetLayout(), };
		GraphicsPipelineBuilder basicPipelineBuilder;
		basicPipelineBuilder.SetDefault()
			.AddShaderStage(*triVert)
			.AddShaderStage(*triFrag)
			.AddViewport(0.f, 0.f, static_cast<float>(windowExtent.width), static_cast<float>(windowExtent.height), 0.0f, 1.0f)
			.AddScissor(0, 0, windowExtent.width, windowExtent.height)
			.SetPipelineLayout(pipelineLayoutCache->Request(descriptorSetLayouts, {}));
		basicPipeline = std::make_unique<Pipeline>("Basic Graphics Pipeline", vulkanContext, basicPipelineBuilder);

		test = std::make_unique<Texture2D>("test", vulkanContext, Extent2D<uint32_t>{ 1280, 720 }, 1, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_LAYOUT_UNDEFINED);
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

				const VkRenderingAttachmentInfoKHR colorAttachmentInfo
				{
					.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR,
					.pNext = nullptr,
					.imageView = swapchainImageView,
					.imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
					.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
					.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
					.clearValue = VkClearValue
					{
						.color = clearColorValue
					}
				};

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
					//graphicsCmdBuffer->BindDescriptorSet(globalDescriptorPool, *basicPipeline);
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
