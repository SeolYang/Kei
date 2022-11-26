#include <Core.h>
#include <Renderer.h>
#include <Window.h>
#include <VK/VulkanInstance.h>
#include <VK/Fence.h>
#include <VK/Semaphore.h>
#include <VK/Swapchain.h>
#include <VK/CommandPool.h>
#include <VK/CommandBuffer.h>

namespace sy
{
	Renderer::Renderer(const Window& window, VulkanInstance& vulkanInstance) :
		window(window),
		vulkanInstance(vulkanInstance)
	{
		for (size_t inFlightFrameIdx = 0; inFlightFrameIdx < NumMaxInFlightFrames; ++inFlightFrameIdx)
		{
			auto& frame = frames[inFlightFrameIdx];
			frame.inFlightFrameIdx = inFlightFrameIdx;
			frame.renderFence = std::make_unique<Fence>(std::format("Render Fence {}", inFlightFrameIdx), vulkanInstance);
			frame.renderSemaphore = std::make_unique<Semaphore>(std::format("Render Semaphore {}", inFlightFrameIdx), vulkanInstance);
			frame.presentSemaphore = std::make_unique<Semaphore>(std::format("Present Semaphore {}", inFlightFrameIdx), vulkanInstance);
		}
	}

	Renderer::~Renderer()
	{
		vulkanInstance.WaitAllQueuesForIdle();
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
		auto graphicsCmdBuffer = graphicsCmdPool.RequestCommandBuffer("Render Cmd Buffer");
		graphicsCmdBufferBatch.emplace_back(*graphicsCmdBuffer);
		graphicsCmdBuffer->Begin();
		{
			const auto graphicsCmdBufferNative = graphicsCmdBuffer->GetNativeHandle();
			const VkImageMemoryBarrier colorAttachmentImgMemoryBarrier
			{
				.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
				.pNext = nullptr,
				.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
				.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
				.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
				.image = swapchainImage,
				.subresourceRange = {
					.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
					.baseMipLevel = 0,
					.levelCount = 1,
					.baseArrayLayer = 0,
					.layerCount = 1,
				}
			};

			vkCmdPipelineBarrier(
				graphicsCmdBufferNative,
				VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
				VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
				0, 0, nullptr, 0, nullptr, 1, &colorAttachmentImgMemoryBarrier);

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
			}
			graphicsCmdBuffer->EndRendering();

			const VkImageMemoryBarrier presentImgMemoryBarrier
			{
				.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
				.pNext = nullptr,
				.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
				.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
				.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
				.image = swapchainImage,
				.subresourceRange = {
					.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
					.baseMipLevel = 0,
					.levelCount = 1,
					.baseArrayLayer = 0,
					.layerCount = 1,
				}
			};

			vkCmdPipelineBarrier(
				graphicsCmdBufferNative,
				VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
				VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
				0, 0, nullptr, 0, nullptr, 1, &presentImgMemoryBarrier);
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
