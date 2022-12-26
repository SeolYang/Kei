#include <Core/Core.h>
#include <Core/Window.h>
#include <Render/Renderer.h>
#include <Render/Vertex.h>
#include <Render/RenderPasses/SimpleRenderPass.h>
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
#include <VK/TextureView.h>
#include <Vk/Sampler.h>
#include <VK/Buffer.h>
#include <VK/DescriptorManager.h>
#include <VK/CommandPoolManager.h>
#include <VK/FrameTracker.h>
#include <Math/MathUtils.h>
#include <Asset/TextureAsset.h>
#include <Asset/MeshAsset.h>

namespace sy
{
	namespace render
	{
		Renderer::Renderer(const Window& window, vk::VulkanContext& vulkanContext, const vk::FrameTracker& frameTracker, vk::CommandPoolManager& cmdPoolManager, vk::DescriptorManager& descriptorManager) :
			window(window),
			vulkanContext(vulkanContext),
			frameTracker(frameTracker),
			cmdPoolManager(cmdPoolManager),
			descriptorManager(descriptorManager),
			pipelineLayoutCache(std::make_unique<vk::PipelineLayoutCache>(vulkanContext))
		{
			const auto windowExtent = window.GetExtent();

			depthStencil = vk::Texture::CreateDepthStencil("Depth-Stencil buffer", vulkanContext, frameTracker, cmdPoolManager, windowExtent);
			depthStencilView = std::make_unique<vk::TextureView>("DepthStencil view", vulkanContext, *depthStencil, VK_IMAGE_VIEW_TYPE_2D);

			triVert = std::make_unique<vk::ShaderModule>("Triangle vertex shader", vulkanContext, "Assets/Shaders/bin/textured_tri_bindless.vert.spv", VK_SHADER_STAGE_VERTEX_BIT, "main");
			triFrag = std::make_unique<vk::ShaderModule>("Triangle fragment shader", vulkanContext, "Assets/Shaders/bin/textured_tri_bindless.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT, "main");

			std::array descriptorSetLayouts = { descriptorManager.GetDescriptorSetLayout(), };
			std::array pushConstantRanges = {
				VkPushConstantRange{ VK_SHADER_STAGE_ALL_GRAPHICS, 0, sizeof(PushConstants) },
			};

			const vk::VertexInputBuilder vertexInputLayout = BuildVertexInputLayout<VertexPTN>();
			vk::GraphicsPipelineBuilder basicPipelineBuilder;
			basicPipelineBuilder.SetDefault()
				.AddShaderStage(*triVert)
				.AddShaderStage(*triFrag)
				.AddViewport(0.f, 0.f, static_cast<float>(windowExtent.width), static_cast<float>(windowExtent.height), 0.0f, 1.0f)
				.AddScissor(0, 0, windowExtent.width, windowExtent.height)
				.SetPipelineLayout(pipelineLayoutCache->Request(descriptorSetLayouts, pushConstantRanges))
				.SetVertexInputLayout(vertexInputLayout);

			basicPipeline = std::make_unique<vk::Pipeline>("Basic Graphics Pipeline", vulkanContext, basicPipelineBuilder);

			linearSampler = std::make_unique<vk::Sampler>("Linear Sampler", vulkanContext);
			loadedTexture = asset::LoadTexture2DFromAsset("Assets/Textures/djmax_1st_anv.tex", vulkanContext, frameTracker, cmdPoolManager);
			loadedTextureView = std::make_unique<vk::TextureView>("Loaded Texture View", vulkanContext, *loadedTexture, VK_IMAGE_VIEW_TYPE_2D);
			loadedTextureDescriptor = descriptorManager.RequestDescriptor(*loadedTexture, *loadedTextureView, *linearSampler, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

			auto [vb, ib] = asset::LoadMeshFromAsset("Assets/Models/box_textured/BoxTextured.mesh", vulkanContext, cmdPoolManager, frameTracker);
			cubeVertexBuffer = std::move(vb);
			cubeIndexBuffer = std::move(ib);

			auto proj = math::PerspectiveYFlipped(glm::radians(45.f), 16.f / 9.f, 0.1f, 1000.f);
			viewProjMat = proj * glm::lookAt(glm::vec3{ 1.5f, 2.f, -5.f }, { 0.f, 0.f ,0.f }, { 0.f ,1.f, 0.f });

			renderPass = std::make_unique<SimpleRenderPass>("Simple Render Pass", vulkanContext, descriptorManager, frameTracker, *basicPipeline);
		}

		Renderer::~Renderer()
		{
			vulkanContext.WaitAllQueuesForIdle();
		}

		void Renderer::Render()
		{
			BeginFrame();
			{
				elapsedTime += 0.01633333f; // hard-coded delta time
				const auto& renderSemaphore = frameTracker.GetCurrentInFlightRenderSemaphore();
				const size_t currentFrameIdx = frameTracker.GetCurrentFrameIndex();

				const auto& swapchain = vulkanContext.GetSwapchain();

				VkClearColorValue clearColorValue;
				clearColorValue.float32[0] = std::cos(static_cast<float>(currentFrameIdx) / 180.f) * 0.5f + 1.f;
				clearColorValue.float32[1] = std::sin(static_cast<float>(currentFrameIdx) / 270.f) * 0.5f + 1.f;
				clearColorValue.float32[2] = std::cos(static_cast<float>(currentFrameIdx) / 90.f) * 0.5f + 1.f;
				clearColorValue.float32[3] = 1.f;

				renderPass->SetVertexBuffer(*cubeVertexBuffer);
				renderPass->SetIndexBuffer(*cubeIndexBuffer);
				renderPass->SetTextureDescriptor(loadedTextureDescriptor);
				renderPass->SetWindowExtent(window.GetExtent());
				renderPass->SetSwapchain(swapchain, clearColorValue);
				renderPass->SetDepthStencilView(*depthStencilView);

				const auto model = glm::rotate(glm::mat4(1.f), elapsedTime, { 0.f, 1.f, 0.f });
				const TransformUniformBuffer uniformBuffer{ viewProjMat * model };
				renderPass->UpdateUniformBuffer(uniformBuffer);

				renderPass->Render(cmdPoolManager);
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
