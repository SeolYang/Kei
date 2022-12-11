#include <Core/Core.h>
#include <Core/Window.h>
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
#include <VK/DescriptorManager.h>
#include <VK/CommandPoolManager.h>
#include <VK/FrameTracker.h>
#include <Render/Renderer.h>
#include <Math/MathUtils.h>
#include <Asset/TextureAsset.h>

namespace sy
{
	namespace render
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

		Renderer::Renderer(const Window& window, vk::VulkanContext& vulkanContext, const vk::FrameTracker& frameTracker, vk::CommandPoolManager& cmdPoolManager, vk::DescriptorManager& descriptorManager) :
			window(window),
			vulkanContext(vulkanContext),
			frameTracker(frameTracker),
			cmdPoolManager(cmdPoolManager),
			descriptorManager(descriptorManager),
			pipelineLayoutCache(std::make_unique<vk::PipelineLayoutCache>(vulkanContext))
		{
			const auto windowExtent = window.GetExtent();

			depthStencil = vk::Texture2D::CreateDepthStencil("Depth-Stencil buffer", vulkanContext, frameTracker, cmdPoolManager, windowExtent);

			triVert = std::make_unique<vk::ShaderModule>("Triangle vertex shader", vulkanContext, "Assets/Shaders/bin/textured_tri_bindless.vert.spv", VK_SHADER_STAGE_VERTEX_BIT, "main");
			triFrag = std::make_unique<vk::ShaderModule>("Triangle fragment shader", vulkanContext, "Assets/Shaders/bin/textured_tri_bindless.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT, "main");

			std::array descriptorSetLayouts = { descriptorManager.GetDescriptorSetLayout(), };
			std::array pushConstantRanges = {
				VkPushConstantRange{ VK_SHADER_STAGE_ALL_GRAPHICS, 0, sizeof(PushConstants) },
			};

			vk::GraphicsPipelineBuilder basicPipelineBuilder;
			basicPipelineBuilder.SetDefault()
				.AddShaderStage(*triVert)
				.AddShaderStage(*triFrag)
				.AddViewport(0.f, 0.f, static_cast<float>(windowExtent.width), static_cast<float>(windowExtent.height), 0.0f, 1.0f)
				.AddScissor(0, 0, windowExtent.width, windowExtent.height)
				.SetPipelineLayout(pipelineLayoutCache->Request(descriptorSetLayouts, pushConstantRanges))
				.AddVertexInputBinding<SimpleVertex>(0, VK_VERTEX_INPUT_RATE_VERTEX)
				.AddVertexInputAttribute(0, 0, VK_FORMAT_R32G32B32A32_SFLOAT, 0)
				.AddVertexInputAttribute(1, 0, VK_FORMAT_R32G32_SFLOAT, static_cast<uint32_t>(offsetof(SimpleVertex, uvs)));

			basicPipeline = std::make_unique<vk::Pipeline>("Basic Graphics Pipeline", vulkanContext, basicPipelineBuilder);

			for (size_t idx = 0; idx < vk::NumMaxInFlightFrames; ++idx)
			{
				transformBuffers[idx] = vk::Buffer::CreateUniformBuffer<TransformUniformBuffer>("TransformBuffer", vulkanContext);
				transformBufferIndices[idx] = descriptorManager.RequestDescriptor(*transformBuffers[idx]);
			}

			loadedTexture = asset::LoadTextureFromAsset("Assets/Textures/djmax_1st_anv.tex", vulkanContext, frameTracker, cmdPoolManager);
			loadedTextureDescriptor = descriptorManager.RequestDescriptor(*loadedTexture);

			//std::array vertices = {
			//	SimpleVertex{glm::vec4{-0.5f, 0.5f, 0.f, 1.f}, {0.f, 1.f} },
			//	SimpleVertex{glm::vec4{-0.5f, -0.5f, 0.f, 1.f}, {0.f, 0.f} },
			//	SimpleVertex{glm::vec4{0.5f, -0.5f, 0.f, 1.f}, {1.f, 0.f} },
			//	SimpleVertex{glm::vec4{0.5f, 0.5f, 0.f, 1.f}, {1.f, 1.f} }
			//};

			std::array vertices =
			{
				SimpleVertex{ {-.5f, -.5f, -.5f, 1.f}, {0.f, 1.f} },
				SimpleVertex{ {-.5f, .5f, -.5f, 1.f}, {0.f, 0.f} },
				SimpleVertex{ {.5f, .5f, -.5f, 1.f}, {1.f, 0.f} },
				SimpleVertex{ {.5f, -.5f, -.5f, 1.f}, {1.f, 1.f} },
				SimpleVertex{ {-.5f, -.5f, .5f, 1.f}, {1.f, 1.f} },
				SimpleVertex{ {-.5f, .5f, .5f, 1.f}, {1.f, 0.f} },
				SimpleVertex{ {.5f, .5f, .5f, 1.f}, {0.f, 0.f} },
				SimpleVertex{ {.5f, -.5f, .5f, 1.f}, {0.f, 1.f} },
			};

			std::array indices = {
				static_cast<uint32_t>(0), // front face
				static_cast<uint32_t>(1),
				static_cast<uint32_t>(2),
				static_cast<uint32_t>(2),
				static_cast<uint32_t>(3),
				static_cast<uint32_t>(0),

				static_cast<uint32_t>(1), // left
				static_cast<uint32_t>(0),
				static_cast<uint32_t>(4),
				static_cast<uint32_t>(4),
				static_cast<uint32_t>(5),
				static_cast<uint32_t>(1),

				static_cast<uint32_t>(7), // right
				static_cast<uint32_t>(3),
				static_cast<uint32_t>(2),
				static_cast<uint32_t>(2),
				static_cast<uint32_t>(6),
				static_cast<uint32_t>(7),

				static_cast<uint32_t>(4), // behind
				static_cast<uint32_t>(7),
				static_cast<uint32_t>(6),
				static_cast<uint32_t>(6),
				static_cast<uint32_t>(5),
				static_cast<uint32_t>(4),

				static_cast<uint32_t>(1), // up
				static_cast<uint32_t>(5),
				static_cast<uint32_t>(6),
				static_cast<uint32_t>(6),
				static_cast<uint32_t>(2),
				static_cast<uint32_t>(1),

				static_cast<uint32_t>(7), // down
				static_cast<uint32_t>(4),
				static_cast<uint32_t>(0),
				static_cast<uint32_t>(0),
				static_cast<uint32_t>(3),
				static_cast<uint32_t>(7),
			};

			cubeVertexBuffer = vk::Buffer::CreateVertexBuffer<SimpleVertex>(cmdPoolManager, frameTracker, "Cube Vertex Buffer", vulkanContext, vertices);
			vulkanContext.SetObjectName(*cubeVertexBuffer);
			cubeIndexBuffer = vk::Buffer::CreateIndexBuffer(cmdPoolManager, frameTracker, "Cube Index Buffer", vulkanContext, indices);
			vulkanContext.SetObjectName(*cubeIndexBuffer);

			auto proj = math::PerspectiveYFlipped(glm::radians(45.f), 16.f / 9.f, 0.1f, 1000.f);
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
				const auto& renderFence = frameTracker.GetCurrentInFlightRenderFence();
				auto& renderSemaphore = frameTracker.GetCurrentInFlightRenderSemaphore();
				auto& presentSemaphore = frameTracker.GetCurrentInFlightPresentSemaphore();
				const size_t currentFrameIdx = frameTracker.GetCurrentFrameIndex();

				const auto windowExtent = window.GetExtent();
				auto& swapchain = vulkanContext.GetSwapchain();
				const auto swapchainImage = swapchain.GetCurrentImage();
				const auto swapchainImageView = swapchain.GetCurrentImageView();

				auto& graphicsCmdPool = cmdPoolManager.RequestCommandPool(vk::EQueueType::Graphics);
				CRefVec<vk::CommandBuffer> graphicsCmdBufferBatch;
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
					clearColorValue.float32[0] = std::cos(static_cast<float>(currentFrameIdx) / 180.f) * 0.5f + 1.f;
					clearColorValue.float32[1] = std::sin(static_cast<float>(currentFrameIdx) / 270.f) * 0.5f + 1.f;
					clearColorValue.float32[2] = std::cos(static_cast<float>(currentFrameIdx) / 90.f) * 0.5f + 1.f;
					clearColorValue.float32[3] = 1.f;

					std::array colorAttachmentInfos = { swapchain.GetColorAttachmentInfo(clearColorValue) };
					std::array depthAttachmentInfos = { vk::DepthAttachmentInfo(*depthStencil) };

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
						graphicsCmdBuffer->BindPipeline(*basicPipeline);
						graphicsCmdBuffer->BindDescriptorSet(descriptorManager.GetDescriptorSet(), *basicPipeline);

						const auto& transformBuffer = *transformBuffers[frameTracker.GetCurrentInFlightFrameIndex()];
						void* transformBufferMappedPtr = vulkanContext.Map(transformBuffer);
						const auto model = glm::rotate(glm::mat4(1.f), elapsedTime, { 0.f, 1.f, 0.f });
						const TransformUniformBuffer uniformBuffer{ viewProjMat * model };
						memcpy(transformBufferMappedPtr, &uniformBuffer, sizeof(TransformUniformBuffer));
						vulkanContext.Unmap(transformBuffer);

						const PushConstants pushConstants
						{
							.textureIndex = static_cast<int>(loadedTextureDescriptor->Offset),
							.transformDataIndex = static_cast<int>(transformBufferIndices[frameTracker.GetCurrentInFlightFrameIndex()]->Offset)
						};

						std::array<CRef<vk::Buffer>, 1> vertexBuffers = { *cubeVertexBuffer };
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

				CRefVec<vk::Semaphore> waitSemaphores;
				waitSemaphores.emplace_back(presentSemaphore);
				CRefVec<vk::Semaphore> signalSemaphores;
				signalSemaphores.emplace_back(renderSemaphore);

				vulkanContext.SubmitTo(
					vk::EQueueType::Graphics,
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
}
