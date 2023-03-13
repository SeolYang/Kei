#include <PCH.h>
#include <Window/Window.h>
#include <Render/Renderer.h>
#include <Render/Vertex.h>
#include <Render/RenderPasses/SimpleRenderPass.h>
#include <VK/VulkanRHI.h>
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
#include <VK/PushConstantBuilder.h>
#include <Math/MathUtils.h>
#include <Asset/TextureAsset.h>
#include <Asset/ModelAsset.h>
#include <Core/ResourceCache.h>

#include "Material.h"
#include "VK/TextureBuilder.h"
#include "VK/VulkanContext.h"

namespace sy::render
{
	Renderer::Renderer(const window::Window& window, vk::VulkanContext& vulkanContext,
	                   vk::ResourceStateTracker& resStateTracker, ResourceCache& resourceCache) :
		window(window),
		vulkanContext(vulkanContext),
		resStateTracker(resStateTracker),
		resourceCache(resourceCache)
	{
		const auto windowExtent   = window.GetExtent();
		const auto& frameTracker  = vulkanContext.GetFrameTracker();
		const auto& vulkanRHI     = vulkanContext.GetRHI();
		auto& cmdPoolManager      = vulkanContext.GetCommandPoolManager();
		auto& descriptorManager   = vulkanContext.GetDescriptorManager();
		auto& pipelineLayoutCache = vulkanContext.GetPipelineLayoutCache();

		depthStencil = vk::TextureBuilder::Texture2DDepthStencilTemplate(vulkanContext)
		               .SetName("Depth-Stencil Buffer")
		               .SetExtent(windowExtent)
		               .SetFormat(VK_FORMAT_D24_UNORM_S8_UINT)
		               .Build();
		depthStencilView = std::make_unique<vk::TextureView>("DepthStencil view", vulkanRHI, *depthStencil,
		                                                     VK_IMAGE_VIEW_TYPE_2D);

		triVert = std::make_unique<vk::ShaderModule>("Triangle vertex shader", vulkanRHI,
		                                             "Assets/Shaders/bin/textured_tri_bindless.vert.spv",
		                                             VK_SHADER_STAGE_VERTEX_BIT, "main");
		triFrag = std::make_unique<vk::ShaderModule>("Triangle fragment shader", vulkanRHI,
		                                             "Assets/Shaders/bin/textured_tri_bindless.frag.spv",
		                                             VK_SHADER_STAGE_FRAGMENT_BIT, "main");

		std::array descriptorSetLayouts = { descriptorManager.GetDescriptorSetLayout(), };

		vk::PushConstantBuilder pushConstantBuilder;
		pushConstantBuilder.Add<PushConstants>(VK_SHADER_STAGE_ALL_GRAPHICS);

		const vk::VertexInputBuilder vertexInputLayout = BuildVertexInputLayout<VertexPTN>();
		vk::GraphicsPipelineBuilder basicPipelineBuilder;
		basicPipelineBuilder.SetDefault()
		                    .AddShaderStage(*triVert)
		                    .AddShaderStage(*triFrag)
		                    .AddViewport(0.f, 0.f, static_cast<float>(windowExtent.width),
		                                 static_cast<float>(windowExtent.height), 0.0f, 1.0f)
		                    .AddScissor(0, 0, windowExtent.width, windowExtent.height)
		                    .SetPipelineLayout(pipelineLayoutCache.Request(descriptorSetLayouts, pushConstantBuilder))
		                    .SetVertexInputLayout(vertexInputLayout);

		basicPipeline = std::make_unique<vk::Pipeline>("Basic Graphics Pipeline", vulkanRHI, basicPipelineBuilder);

		staticMeshes = asset::LoadModel("Homura", "Assets/Models/homura/homura_v1.MODEL", resourceCache, vulkanContext);
		const auto proj = math::PerspectiveYFlipped(glm::radians(45.f), 16.f / 9.f, 0.1f, 1000.f);
		viewProjMat = proj * glm::lookAt(glm::vec3{ 1.5f, 160.f, -150.f }, { 0.f, 70.f, 0.f }, { 0.f, 1.f, 0.f });

		renderPass = std::make_unique<SimpleRenderPass>("Simple Render Pass", resourceCache, vulkanContext,
		                                                *basicPipeline);
	}

	Renderer::~Renderer()
	{
		/* Empty */
	}

	void Renderer::Render()
	{
		const auto& frameTracker = vulkanContext.GetFrameTracker();
		const auto& vulkanRHI    = vulkanContext.GetRHI();
		BeginFrame();
		{
			elapsedTime += 0.00833333f; // hard-coded delta time
			const auto& renderSemaphore  = frameTracker.GetCurrentInFlightRenderSemaphore();
			const size_t currentFrameIdx = frameTracker.GetCurrentFrameIndex();

			const auto& swapchain = vulkanRHI.GetSwapchain();

			VkClearColorValue clearColorValue;
			clearColorValue.float32[ 0 ] = 0.f;
			clearColorValue.float32[ 1 ] = 0.f;
			clearColorValue.float32[ 2 ] = 0.f;
			clearColorValue.float32[ 3 ] = 1.f;

			renderPass->SetWindowExtent(window.GetExtent());
			renderPass->SetSwapchain(swapchain, clearColorValue);
			renderPass->SetDepthStencilView(*depthStencilView);

			const auto model = glm::rotate(glm::rotate(glm::mat4(1.f), glm::radians(-90.f), { 1.f, 0.f, 0.f }),
			                               elapsedTime, { 0.f, 0.f, 1.f });
			renderPass->SetTransformData({ viewProjMat * model });
			renderPass->UpdateBuffers();

			CRefVec<vk::CommandBuffer> batchedCmdBuffers;

			renderPass->Begin(vk::EQueueType::Graphics);
			for (const auto& mesh : staticMeshes)
			{
				const auto& materialRef = Unwrap(resourceCache.Load(mesh.Material));
				renderPass->SetMesh(mesh.Mesh);
				renderPass->SetTextureDescriptor(materialRef.BaseTexture);
				renderPass->Render();
			}
			renderPass->End();

			batchedCmdBuffers.emplace_back(renderPass->GetCommandBuffer());

			const auto& renderFence = frameTracker.GetCurrentInFlightRenderFence();
			auto& presentSemaphore  = frameTracker.GetCurrentInFlightPresentSemaphore();

			CRefVec<vk::Semaphore> waitSemaphores;
			waitSemaphores.emplace_back(presentSemaphore);
			CRefVec<vk::Semaphore> signalSemaphores;
			signalSemaphores.emplace_back(renderSemaphore);

			vulkanRHI.SubmitTo(
			                   vk::EQueueType::Graphics,
			                   frameTracker,
			                   batchedCmdBuffers);

			vulkanRHI.Present(swapchain, renderSemaphore);
		}
		EndFrame();
	}

	void Renderer::BeginFrame()
	{
		const auto& vulkanRHI    = vulkanContext.GetRHI();
		const auto& frameTracker = vulkanContext.GetFrameTracker();
		auto& swapchain          = vulkanRHI.GetSwapchain();
		swapchain.AcquireNext(frameTracker.GetCurrentInFlightPresentSemaphore());
		frameTracker.WaitForInFlightRenderFence();
		frameTracker.ResetInFlightRenderFence();
	}

	void Renderer::EndFrame()
	{
		/* Empty */
	}
}
