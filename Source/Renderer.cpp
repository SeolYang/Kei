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
	struct TransformUniformBuffer
	{
		glm::mat4 modelViewProj;
	};

	struct PushConstants
	{
		int textureIndex;
		int transformDataIndex;
	};

	struct SimpleVertex
	{
		glm::vec4 pos;
		glm::vec2 uvs;
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

		depthStencil = Texture2D::CreateDepthStencil(cmdPoolManager, frameTracker, "Depth-Stencil buffer", vulkanContext, windowExtent);

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
			.SetPipelineLayout(pipelineLayoutCache->Request(descriptorSetLayouts, pushConstantRanges))
			.AddVertexInputBinding<SimpleVertex>(0, VK_VERTEX_INPUT_RATE_VERTEX)
			.AddVertexInputAttribute(0, 0, VK_FORMAT_R32G32B32A32_SFLOAT, 0)
			.AddVertexInputAttribute(1, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(SimpleVertex, uvs));

		basicPipeline = std::make_unique<Pipeline>("Basic Graphics Pipeline", vulkanContext, basicPipelineBuilder);

		for (size_t idx = 0; idx < NumMaxInFlightFrames; ++idx)
		{
			transformBuffers[idx] = Buffer::CreateUniformBuffer<TransformUniformBuffer>("TransformBuffer", vulkanContext);
			transformBufferIndices[idx] = descriptorManager.RequestDescriptor(*transformBuffers[idx]);
		}

		loadedTexture = Texture2D::LoadFromFile(cmdPoolManager, frameTracker, "Assets/Textures/djmax_1st_anv.png", vulkanContext, VK_FORMAT_R8G8B8A8_SRGB);
		loadedTextureDescriptor = descriptorManager.RequestDescriptor(*loadedTexture);

		//std::array vertices = {
		//	SimpleVertex{glm::vec4{-0.5f, 0.5f, 0.f, 1.f}, {0.f, 1.f} },
		//	SimpleVertex{glm::vec4{-0.5f, -0.5f, 0.f, 1.f}, {0.f, 0.f} },
		//	SimpleVertex{glm::vec4{0.5f, -0.5f, 0.f, 1.f}, {1.f, 0.f} },
		//	SimpleVertex{glm::vec4{0.5f, 0.5f, 0.f, 1.f}, {1.f, 1.f} }
		//};

		std::array vertices =
		{
			SimpleVertex{ {-.5f, .5f, -.5f, 1.f}, {0.f, 1.f} },
			SimpleVertex{ {-.5f, -.5f, -.5f, 1.f}, {0.f, 0.f} },
			SimpleVertex{ {.5f, -.5f, -.5f, 1.f}, {1.f, 0.f} },
			SimpleVertex{ {.5f, .5f, -.5f, 1.f}, {1.f, 1.f} },
			SimpleVertex{ {-.5f, .5f, .5f, 1.f}, {1.f, 1.f} },
			SimpleVertex{ {-.5f, -.5f, .5f, 1.f}, {1.f, 0.f} },
			SimpleVertex{ {.5f, -.5f, .5f, 1.f}, {0.f, 0.f} },
			SimpleVertex{ {.5f, .5f, .5f, 1.f}, {0.f, 1.f} },
		};

		std::array indices = {
			static_cast<uint32_t>(2), // front face
			static_cast<uint32_t>(1),
			static_cast<uint32_t>(0),
			static_cast<uint32_t>(0),
			static_cast<uint32_t>(3),
			static_cast<uint32_t>(2),

			static_cast<uint32_t>(4), // left
			static_cast<uint32_t>(0),
			static_cast<uint32_t>(1),
			static_cast<uint32_t>(1),
			static_cast<uint32_t>(5),
			static_cast<uint32_t>(4),

			static_cast<uint32_t>(2), // right
			static_cast<uint32_t>(3),
			static_cast<uint32_t>(7),
			static_cast<uint32_t>(7),
			static_cast<uint32_t>(6),
			static_cast<uint32_t>(2),

			static_cast<uint32_t>(6), // behind
			static_cast<uint32_t>(7),
			static_cast<uint32_t>(4),
			static_cast<uint32_t>(4),
			static_cast<uint32_t>(5),
			static_cast<uint32_t>(6),

			static_cast<uint32_t>(6), // up
			static_cast<uint32_t>(5),
			static_cast<uint32_t>(1),
			static_cast<uint32_t>(1),
			static_cast<uint32_t>(2),
			static_cast<uint32_t>(6),

			static_cast<uint32_t>(0), // down
			static_cast<uint32_t>(4),
			static_cast<uint32_t>(7),
			static_cast<uint32_t>(7),
			static_cast<uint32_t>(3),
			static_cast<uint32_t>(0),
		};

		cubeVertexBuffer = Buffer::CreateVertexBuffer<SimpleVertex>(cmdPoolManager, frameTracker, "Quad Vertex Buffer", vulkanContext, vertices);
		cubeIndexBuffer = Buffer::CreateIndexBuffer(cmdPoolManager, frameTracker, "Quad Index Buffer", vulkanContext, indices);

		const auto proj = glm::perspective(glm::radians(45.f), 16.f / 9.f, 0.1f, 1000.f);
		viewProjMat = proj * glm::lookAt(glm::vec3{ 1.5f, -2.f, -5.f }, { 0.f, 0.f ,0.f }, { 0.f ,1.f, 0.f });
	}

	Renderer::~Renderer()
	{
		vulkanContext.WaitAllQueuesForIdle();
	}

	void Renderer::Render()
	{
		BeginFrame();
		{
			elapsedTime += 0.01633333f; // hard-coded
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

				std::array colorAttachmentInfos = { swapchain.GetColorAttachmentInfo(clearColorValue) };
				std::array depthAttachmentInfos = { vkinit::DepthAttachmentInfo(*depthStencil) };

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
					.colorAttachmentCount = colorAttachmentInfos.size(),
					.pColorAttachments = colorAttachmentInfos.data(),
					.pDepthAttachment = depthAttachmentInfos.data(),
					.pStencilAttachment = depthAttachmentInfos.data()
				};

				graphicsCmdBuffer->BeginRendering(renderingInfo);
				{
					// Rendering something here
					graphicsCmdBuffer->BindPipeline(*basicPipeline);
					graphicsCmdBuffer->BindDescriptorSet(descriptorManager.GetDescriptorSet(), *basicPipeline);

					const auto& transformBuffer = *transformBuffers[frameTracker.GetCurrentInFlightFrameIndex()];
					void* transformBufferMappedPtr = vulkanContext.Map(transformBuffer);
					const TransformUniformBuffer uniformBuffer{ viewProjMat * glm::rotate(glm::mat4(1.f), elapsedTime, { 0.f, 1.f, 0.f }) };
					memcpy(transformBufferMappedPtr, &uniformBuffer, sizeof(TransformUniformBuffer));
					vulkanContext.Unmap(transformBuffer);

					const PushConstants pushConstants
					{
						.textureIndex = static_cast<int>(loadedTextureDescriptor->Offset),
						.transformDataIndex = static_cast<int>(transformBufferIndices[frameTracker.GetCurrentInFlightFrameIndex()]->Offset)
					};

					std::array<CRef<Buffer>, 1> vertexBuffers = { *cubeVertexBuffer };
					std::array offsets = { uint64_t() };
					graphicsCmdBuffer->BindVertexBuffers(0, vertexBuffers, offsets);
					graphicsCmdBuffer->BindIndexBuffer(*cubeIndexBuffer);
					graphicsCmdBuffer->PushConstants(*basicPipeline, VK_SHADER_STAGE_ALL_GRAPHICS, 0, sizeof(PushConstants), &pushConstants);

					graphicsCmdBuffer->DrawIndexed(36, 1, 0, 0, 0);
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
