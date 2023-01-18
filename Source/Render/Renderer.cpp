#include <PCH.h>
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
#include <VK/PushConstantBuilder.h>
#include <Math/MathUtils.h>
#include <Asset/TextureAsset.h>
#include <Asset/ModelAsset.h>
#include <Core/ResourceCache.h>

namespace sy
{
	namespace render
	{
		Renderer::Renderer(const Window& window, vk::VulkanContext& vulkanContext, vk::ResourceStateTracker& resStateTracker, const vk::FrameTracker& frameTracker, vk::CommandPoolManager& cmdPoolManager, vk::DescriptorManager& descriptorManager, ResourceCache& resourceCache) :
			window(window),
			vulkanContext(vulkanContext),
			resStateTracker(resStateTracker),
			frameTracker(frameTracker),
			cmdPoolManager(cmdPoolManager),
			descriptorManager(descriptorManager),
			resourceCache(resourceCache),
			pipelineLayoutCache(std::make_unique<vk::PipelineLayoutCache>(vulkanContext))
		{
			const auto windowExtent = window.GetExtent();

			depthStencil = vk::CreateDepthStencil("Depth-Stencil buffer", vulkanContext, cmdPoolManager, windowExtent, VK_FORMAT_D24_UNORM_S8_UINT);
			depthStencilView = std::make_unique<vk::TextureView>("DepthStencil view", vulkanContext, *depthStencil, VK_IMAGE_VIEW_TYPE_2D);

			triVert = std::make_unique<vk::ShaderModule>("Triangle vertex shader", vulkanContext, "Assets/Shaders/bin/textured_tri_bindless.vert.spv", VK_SHADER_STAGE_VERTEX_BIT, "main");
			triFrag = std::make_unique<vk::ShaderModule>("Triangle fragment shader", vulkanContext, "Assets/Shaders/bin/textured_tri_bindless.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT, "main");

			std::array descriptorSetLayouts = { descriptorManager.GetDescriptorSetLayout(), };

			vk::PushConstantBuilder pushConstantBuilder;
			pushConstantBuilder.Add<PushConstants>(VK_SHADER_STAGE_ALL_GRAPHICS);

			const vk::VertexInputBuilder vertexInputLayout = BuildVertexInputLayout<VertexPTN>();
			vk::GraphicsPipelineBuilder basicPipelineBuilder;
			basicPipelineBuilder.SetDefault()
				.AddShaderStage(*triVert)
				.AddShaderStage(*triFrag)
				.AddViewport(0.f, 0.f, static_cast<float>(windowExtent.width), static_cast<float>(windowExtent.height), 0.0f, 1.0f)
				.AddScissor(0, 0, windowExtent.width, windowExtent.height)
				.SetPipelineLayout(pipelineLayoutCache->Request(descriptorSetLayouts, pushConstantBuilder))
				.SetVertexInputLayout(vertexInputLayout);

			basicPipeline = std::make_unique<vk::Pipeline>("Basic Graphics Pipeline", vulkanContext, basicPipelineBuilder);

			linearSampler = resourceCache.Add(std::make_unique<vk::Sampler>("Linear Sampler", vulkanContext, vk::SamplerInfo{}));
			auto& linearSamplerRef = Unwrap(resourceCache.Load(linearSampler));

			Handle<vk::Texture> bodyTex = resourceCache.Add(asset::LoadTexture2DFromAsset("Assets/Textures/Body.TEX", vulkanContext, frameTracker, cmdPoolManager));
			auto& bodyTexRef = Unwrap(resourceCache.Load(bodyTex));
			/** @todo texture view constructor ∞£º“»≠ */
			Handle<vk::TextureView> bodyTexView = resourceCache.Add(std::make_unique<vk::TextureView>("BodyTextureView", vulkanContext, bodyTexRef, VK_IMAGE_VIEW_TYPE_2D, bodyTexRef.GetFormat(), vk::FormatToImageAspect(bodyTexRef.GetFormat())));
			bodyTexDescriptor = resourceCache.Add(std::make_unique<vk::Descriptor>(descriptorManager.RequestDescriptor(bodyTexRef, Unwrap(resourceCache.Load(bodyTexView)), linearSamplerRef, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)));

			Handle<vk::Texture> hairTex = resourceCache.Add(asset::LoadTexture2DFromAsset("Assets/Textures/Hair.TEX", vulkanContext, frameTracker, cmdPoolManager));
			auto& hairTexRef = Unwrap(resourceCache.Load(hairTex));
			Handle<vk::TextureView> hairTexView = resourceCache.Add(std::make_unique<vk::TextureView>("HairTextureView", vulkanContext, hairTexRef, VK_IMAGE_VIEW_TYPE_2D, hairTexRef.GetFormat(), vk::FormatToImageAspect(hairTexRef.GetFormat())));
			hairTexDescriptor = resourceCache.Add(std::make_unique<vk::Descriptor>(descriptorManager.RequestDescriptor(hairTexRef, Unwrap(resourceCache.Load(hairTexView)), linearSamplerRef, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)));

			Handle<vk::Texture> costumeTex = resourceCache.Add(asset::LoadTexture2DFromAsset("Assets/Textures/Costume.TEX", vulkanContext, frameTracker, cmdPoolManager));
			auto& costumeTexRef = Unwrap(resourceCache.Load(costumeTex));
			Handle<vk::TextureView> costumeTexView = resourceCache.Add(std::make_unique<vk::TextureView>("CostumeTextureView", vulkanContext, costumeTexRef, VK_IMAGE_VIEW_TYPE_2D, costumeTexRef.GetFormat(), vk::FormatToImageAspect(costumeTexRef.GetFormat())));
			costumeTexDescriptor = resourceCache.Add(std::make_unique<vk::Descriptor>(descriptorManager.RequestDescriptor(costumeTexRef, Unwrap(resourceCache.Load(costumeTexView)), linearSamplerRef, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)));

			auto loadedMeshes = asset::LoadMeshesFromModelAsset("Assets/Models/homura/homura.model", vulkanContext, cmdPoolManager, frameTracker);
			for (auto& mesh : loadedMeshes)
			{
				meshHandles.emplace_back(resourceCache.Add(std::move(mesh)));
			}
			loadedMeshes.clear();

			auto proj = math::PerspectiveYFlipped(glm::radians(45.f), 16.f / 9.f, 0.1f, 1000.f);
			viewProjMat = proj * glm::lookAt(glm::vec3{ 1.5f, 160.f, -150.f }, { 0.f, 70.f ,0.f }, { 0.f ,1.f, 0.f });

			renderPass = std::make_unique<SimpleRenderPass>("Simple Render Pass", resourceCache, vulkanContext, descriptorManager, frameTracker, cmdPoolManager, *basicPipeline);
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

				renderPass->SetWindowExtent(window.GetExtent());
				renderPass->SetSwapchain(swapchain, clearColorValue);
				renderPass->SetDepthStencilView(*depthStencilView);

				const auto model = glm::rotate(glm::rotate(glm::mat4(1.f), glm::radians(-90.f), { 1.f, 0.f, 0.f }), elapsedTime, {0.f, 0.f, 1.f});
				renderPass->SetTransformData({ viewProjMat * model });
				renderPass->UpdateBuffers();

				CRefVec<vk::CommandBuffer> batchedCmdBuffers;

				renderPass->Begin(vk::EQueueType::Graphics);

				for (const auto& mesh : meshHandles)
				{
					const auto& meshRef = Unwrap(resourceCache.Load(mesh));
					if (meshRef.GetName().contains("Hair") || meshRef.GetName().contains("Kemono") || meshRef.GetName().contains("Twin"))
					{
						renderPass->SetTextureDescriptor(hairTexDescriptor);
					}
					else if (meshRef.GetName().contains("Body"))
					{
						renderPass->SetTextureDescriptor(bodyTexDescriptor);
					}
					else
					{
						renderPass->SetTextureDescriptor(costumeTexDescriptor);
					}

					renderPass->SetMesh(mesh);
					renderPass->Render();
				}
				renderPass->End();

				batchedCmdBuffers.emplace_back(renderPass->GetCommandBuffer());

				const auto& renderFence = frameTracker.GetCurrentInFlightRenderFence();
				auto& presentSemaphore = frameTracker.GetCurrentInFlightPresentSemaphore();

				CRefVec<vk::Semaphore> waitSemaphores;
				waitSemaphores.emplace_back(presentSemaphore);
				CRefVec<vk::Semaphore> signalSemaphores;
				signalSemaphores.emplace_back(renderSemaphore);

				vulkanContext.SubmitTo(
					vk::EQueueType::Graphics,
					frameTracker,
					batchedCmdBuffers);

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
