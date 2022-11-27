#include <Core.h>
#include <Renderer.h>
#include <Window.h>
#include <VK/VulkanInstance.h>
#include <VK/Fence.h>
#include <VK/Semaphore.h>
#include <VK/Swapchain.h>
#include <VK/CommandPool.h>
#include <VK/CommandBuffer.h>
#include <VK/ShaderModule.h>
#include <VK/Pipeline.h>
#include <VK/PipelineBuilder.h>

namespace sy
{
	Renderer::Renderer(const Window& window, VulkanInstance& vulkanInstance) :
		window(window),
		vulkanInstance(vulkanInstance)
	{
		const auto windowExtent = window.GetExtent();
		for (size_t inFlightFrameIdx = 0; inFlightFrameIdx < NumMaxInFlightFrames; ++inFlightFrameIdx)
		{
			auto& frame = frames[inFlightFrameIdx];
			frame.inFlightFrameIdx = inFlightFrameIdx;
			frame.renderFence = std::make_unique<Fence>(std::format("Render Fence {}", inFlightFrameIdx), vulkanInstance);
			frame.renderSemaphore = std::make_unique<Semaphore>(std::format("Render Semaphore {}", inFlightFrameIdx), vulkanInstance);
			frame.presentSemaphore = std::make_unique<Semaphore>(std::format("Present Semaphore {}", inFlightFrameIdx), vulkanInstance);
		}

		triVert = std::make_unique<ShaderModule>("Triangle vertex shader", vulkanInstance, "Assets/Shaders/bin/tri.vert.spv", VK_SHADER_STAGE_VERTEX_BIT, "main");
		triFrag = std::make_unique<ShaderModule>("Triangle fragment shader", vulkanInstance, "Assets/Shaders/bin/tri.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT, "main");

		const VkPipelineLayoutCreateInfo nullLayoutInfo
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.setLayoutCount = 0,
			.pSetLayouts = nullptr,
			.pushConstantRangeCount = 0,
			.pPushConstantRanges = nullptr
		};

		vkCreatePipelineLayout(vulkanInstance.GetLogicalDevice(), &nullLayoutInfo, nullptr, &nullLayout);

		GraphicsPipelineBuilder basicPipelineBuilder;
		basicPipelineBuilder.SetDefault()
			.AddShaderStage(*triVert)
			.AddShaderStage(*triFrag)
			.AddViewport(0.f, 0.f, static_cast<float>(windowExtent.width), static_cast<float>(windowExtent.height), 0.0f, 1.0f)
			.AddScissor(0, 0, windowExtent.width, windowExtent.height)
			.SetPipelineLayout(nullLayout);
		basicPipeline = std::make_unique<Pipeline>("Basic Graphics Pipeline", vulkanInstance, basicPipelineBuilder);
	}

	Renderer::~Renderer()
	{
		vulkanInstance.WaitAllQueuesForIdle();
		vkDestroyPipelineLayout(vulkanInstance.GetLogicalDevice(), nullLayout, nullptr);
	}

	void Renderer::Render()
	{
		const Frame& frame = BeginFrame();
		const auto windowExtent = window.GetExtent();
		auto& swapchain = vulkanInstance.GetSwapchain();
		const auto swapchainImage = swapchain.GetCurrentImage();
		const auto swapchainImageView = swapchain.GetCurrentImageView();

		auto& graphicsCmdPool = vulkanInstance.RequestCommandPool(EQueueType::Graphics, GetCurrentInFlightFrameIndex());
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
		waitSemaphores.emplace_back(*frame.presentSemaphore);
		CRefVec<Semaphore> signalSemaphores;
		signalSemaphores.emplace_back(*frame.renderSemaphore);

		vulkanInstance.SubmitTo(EQueueType::Graphics,
			waitSemaphores,
			graphicsCmdBufferBatch,
			signalSemaphores,
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, *frame.renderFence);

		vulkanInstance.Present(swapchain, *frame.renderSemaphore);
		EndFrame(frame);
	}

	const Frame& Renderer::BeginFrame()
	{
		const auto& currentFrame = GetCurrentInFlightFrame();
		auto& swapchain = vulkanInstance.GetSwapchain();
		swapchain.AcquireNext(*currentFrame.presentSemaphore);
		currentFrame.renderFence->Wait();
		currentFrame.renderFence->Reset();
		vulkanInstance.BeginFrame(GetCurrentInFlightFrameIndex());

		return currentFrame;
	}

	void Renderer::EndFrame(const Frame& currentFrame)
	{
		++currentFrameIdx;
	}
}
