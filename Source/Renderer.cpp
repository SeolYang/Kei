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
		vulkanInstance(vulkanInstance),
		renderSemaphore(std::make_unique<Semaphore>("Render Semaphore", vulkanInstance)),
		renderFence(std::make_unique<Fence>("Render Fence", vulkanInstance))
	{
	}

	Renderer::~Renderer()
	{
		renderFence->Wait();
		vulkanInstance.WaitAllQueuesForIdle();
		renderSemaphore.reset();
	}

	void Renderer::Render()
	{
		++currentFrames;

		const auto windowExtent = window.GetExtent();
		auto& graphicsCmdPool = vulkanInstance.RequestGraphicsCommandPool();

		auto& swapchain = vulkanInstance.GetSwapchain();
		const auto& presentSemaphore = swapchain.GetSemaphore();
		swapchain.AcquireNext();
		const auto swapchainImage = swapchain.GetCurrentImage();
		const auto swapchainImageView = swapchain.GetCurrentImageView();

		const auto& graphicsCmdBuffer = graphicsCmdPool.RequestCommandBuffer("Render Cmd Buffer", *renderFence);
		const auto graphicsCmdBufferNative = graphicsCmdBuffer.GetNativeHandle();

		renderFence->Wait();
		renderFence->Reset();
		{
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
			clearColorValue.float32[0] = std::cos((float)currentFrames / 180.0f);
			clearColorValue.float32[1] = std::sin((float)currentFrames / 180.0f);
			clearColorValue.float32[2] = std::cos((float)currentFrames / 180.0f);
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

			vkCmdBeginRendering(graphicsCmdBufferNative, &renderingInfo);
			{
				// Rendering something here
			}
			vkCmdEndRendering(graphicsCmdBufferNative);

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
		graphicsCmdBuffer.End();

		const auto presentVkSemaphore = presentSemaphore.GetNativeHandle();
		const auto renderVkSemaphore = renderSemaphore->GetNativeHandle();
		constexpr VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		const VkSubmitInfo submitInfo
		{
			.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
			.pNext = nullptr,
			.waitSemaphoreCount = 1,
			.pWaitSemaphores = &presentVkSemaphore,
			.pWaitDstStageMask = &waitStage,
			.commandBufferCount = 1,
			.pCommandBuffers = &graphicsCmdBufferNative,
			.signalSemaphoreCount = 1,
			.pSignalSemaphores = &renderVkSemaphore,
		};

		vulkanInstance.SubmitTo(EQueueType::Graphics, submitInfo, *renderFence);

		VkPresentInfoKHR presentInfo = {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.pNext = nullptr;

		const auto swapchainNative = swapchain.GetNativeHandle();
		presentInfo.pSwapchains = &swapchainNative;
		presentInfo.swapchainCount = 1;

		presentInfo.pWaitSemaphores = &renderVkSemaphore;
		presentInfo.waitSemaphoreCount = 1;

		const auto swapchainImageIndex = swapchain.GetCurrentImageIndex();
		presentInfo.pImageIndices = &swapchainImageIndex;

		vulkanInstance.Present(presentInfo);
	}
}
