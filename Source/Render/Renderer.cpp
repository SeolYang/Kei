#include <PCH.h>
#include <Render/Renderer.h>
#include <Render/Material.h>
#include <Render/Vertex.h>
#include <Render/RenderPasses/SimpleRenderPass.h>
#include <Render/RenderGraph.h>
#include <Render/RenderNode.h>
#include <VK/VulkanContext.h>
#include <VK/VulkanRHI.h>
#include <VK/Semaphore.h>
#include <VK/Swapchain.h>
#include <VK/CommandBuffer.h>
#include <VK/ShaderModule.h>
#include <VK/Pipeline.h>
#include <VK/PipelineBuilder.h>
#include <VK/LayoutCache.h>
#include <VK/TextureView.h>
#include <VK/DescriptorAllocator.h>
#include <VK/FrameTracker.h>
#include <VK/PushConstantBuilder.h>
#include <VK/Texture.h>
#include <VK/TextureBuilder.h>
#include <Window/Window.h>
#include <Math/MathUtils.h>
#include <Asset/ModelAsset.h>

namespace sy::render
{
Renderer::Renderer(const window::Window& window, vk::VulkanContext& vulkanContext, HandleManager& handleManager) :
    window(window), vulkanContext(vulkanContext), handleManager(handleManager)
{
}

Renderer::~Renderer()
{
    /** Empty */
}

void Renderer::Render()
{
    auto& frameTracker = vulkanContext.GetFrameTracker();
    const auto& vulkanRHI = vulkanContext.GetRHI();
    {
        elapsedTime += 0.00833333f; // hard-coded delta time
        const size_t frameCounter = frameTracker.GetFrameCounter();

        const auto& swapchain = vulkanContext.GetSwapchain();

        VkClearColorValue clearColorValue;
        clearColorValue.float32[0] = 0.f;
        clearColorValue.float32[1] = 0.f;
        clearColorValue.float32[2] = 0.f;
        clearColorValue.float32[3] = 1.f;

        renderPass->SetWindowExtent(window.GetExtent());
        renderPass->SetSwapchain(swapchain, clearColorValue);
        renderPass->SetDepthStencilView(*depthStencilView);

        const auto model = glm::rotate(glm::mat4(1.f), elapsedTime, {0.f, 1.f, 0.f});
        renderPass->SetTransformData({viewProjMat * model});
        renderPass->UpdateBuffers();

        CRefVec<vk::CommandBuffer> batchedCmdBuffers;

        renderPass->Begin(vk::EQueueType::Graphics);
        for (const auto& mesh : staticMeshes)
        {
            renderPass->SetMesh(mesh);
            renderPass->SetTextureDescriptor(mesh->GetMaterial()->BaseTexture);
            renderPass->Render();
        }
        renderPass->End();

        batchedCmdBuffers.emplace_back(renderPass->GetCommandBuffer());

        CRefArray<vk::Semaphore, 1> waitSemaphores = {frameTracker.GetInflightSwapchainSemaphore()};
        RefArray<vk::Semaphore, 2> signalSemaphores = {frameTracker.GetInflightCommandExecutionSemaphore(), frameTracker.GetInflightPresentSemaphore()};

        vulkanRHI.SubmitSync(vk::EQueueType::Graphics,
                             batchedCmdBuffers,
                             waitSemaphores, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                             signalSemaphores, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);

        // todo: split out (renderSemaphore, and renderNodeSemaphore)
        // submit renderSemaphore at end of render graph(render nodes)
        vulkanRHI.Present(swapchain, signalSemaphores[1]);
    }
}

void Renderer::BeginFrame()
{
    const auto& vulkanRHI = vulkanContext.GetRHI();
    auto& frameTracker = vulkanContext.GetFrameTracker();
    auto& swapchain = vulkanContext.GetSwapchain();
    swapchain.AcquireNext(frameTracker.GetInflightSwapchainSemaphore());

    const auto& cmdExecutionSemaphore = frameTracker.GetInflightCommandExecutionSemaphore();
    cmdExecutionSemaphore.Wait();
}

void Renderer::EndFrame()
{
    /* Empty */
}

void Renderer::Startup()
{
    spdlog::info("Startup Renderer.");
    const auto windowExtent = window.GetExtent();
    const auto& frameTracker = vulkanContext.GetFrameTracker();
    const auto& vulkanRHI = vulkanContext.GetRHI();
    auto& cmdPoolAllocator = vulkanContext.GetCommandPoolAllocator();
    auto& descriptorAllocator = vulkanContext.GetDescriptorAllocator();
    auto& pipelineLayoutCache = vulkanContext.GetPipelineLayoutCache();

    depthStencil = vk::TextureBuilder::Texture2DDepthStencilTemplate(vulkanContext)
                       .SetName("Depth-Stencil Buffer")
                       .SetExtent(windowExtent)
                       .SetFormat(VK_FORMAT_D24_UNORM_S8_UINT)
                       .Build();

    depthStencilView = std::make_unique<vk::TextureView>(
        "DepthStencil view",
        vulkanContext, *depthStencil,
        VK_IMAGE_VIEW_TYPE_2D);

    triVert = std::make_unique<vk::ShaderModule>(
        "Triangle vertex shader",
        vulkanContext,
        "Assets/Shaders/bin/textured_tri_bindless.vert.spv",
        VK_SHADER_STAGE_VERTEX_BIT,
        "main");

    triFrag = std::make_unique<vk::ShaderModule>(
        "Triangle fragment shader",
        vulkanContext,
        "Assets/Shaders/bin/textured_tri_bindless.frag.spv",
        VK_SHADER_STAGE_FRAGMENT_BIT,
        "main");

    std::array descriptorSetLayouts = {
        descriptorAllocator.GetDescriptorSetLayout(),
    };

    vk::PushConstantBuilder pushConstantBuilder;
    pushConstantBuilder.Add<PushConstants>(VK_SHADER_STAGE_ALL_GRAPHICS);

    const vk::VertexInputBuilder vertexInputLayout = BuildVertexInputLayout<VertexPT0N>();
    vk::GraphicsPipelineBuilder basicPipelineBuilder;
    basicPipelineBuilder.SetDefault()
        .AddShaderStage(*triVert)
        .AddShaderStage(*triFrag)
        .AddViewport(0.f, 0.f, static_cast<float>(windowExtent.width),
                     static_cast<float>(windowExtent.height), 0.0f, 1.0f)
        .AddScissor(0, 0, windowExtent.width, windowExtent.height)
        .SetPipelineLayout(pipelineLayoutCache.Request(descriptorSetLayouts, pushConstantBuilder))
        .SetVertexInputLayout(vertexInputLayout);

    basicPipeline = std::make_unique<vk::Pipeline>("Basic Graphics Pipeline", vulkanContext, basicPipelineBuilder);

    //auto model = handleManager.Add<asset::Model>("Assets/Models/rubber_duck/scene.gltf", handleManager, vulkanContext);
    auto model = handleManager.Add<asset::Model>("Assets/Models/homura/homura.fbx", handleManager, vulkanContext);
    SY_ASSERT(model->Initialize(), "Failed to init model.");
    staticMeshes = model->GetMeshes();

    const auto proj = glm::perspective(glm::radians(90.f), 16.f / 9.f, 0.1f, 1000.f);
    viewProjMat = proj * glm::lookAt(glm::vec3{0, 100.f, -80.f}, {0.f, 80.0f, 0.f}, {0.f, 1.f, 0.f});

    renderPass = std::make_unique<SimpleRenderPass>("Simple Render Pass", vulkanContext, *basicPipeline);

    /** todo: remove test codes **/
    auto renderGraph = std::make_unique<RenderGraph>(vulkanContext);
    auto node0 = std::make_unique<RenderNode>(*renderGraph, "n0_graphics");
    node0->CreateTexture("g0");

    auto node1 = std::make_unique<RenderNode>(*renderGraph, "n1_graphics");
    node1->AsGenaralSampledImage("g0");
    node1->CreateTexture("g1");

    auto node2 = std::make_unique<RenderNode>(*renderGraph, "n2_graphics");
    node2->AsGenaralSampledImage("g1");
    node2->AsGenaralSampledImage("g0");
    node2->CreateTexture("g2");

    auto node3 = std::make_unique<RenderNode>(*renderGraph, "n3_asyncCompute");
    node3->CreateTexture("c0");
    node3->ExecuteOnAsyncCompute();

    auto node4 = std::make_unique<RenderNode>(*renderGraph, "n4_asyncCompute");
    node4->CreateTexture("c1");
    node4->AsGenaralSampledImage("c0");
    node4->AsGenaralSampledImage("g0");
    node4->ExecuteOnAsyncCompute();

    auto node5 = std::make_unique<RenderNode>(*renderGraph, "n5_asyncCompute");
    node5->CreateTexture("c2");
    node5->AsGenaralSampledImage("c1");
    node5->AsGenaralSampledImage("g0");
    node5->ExecuteOnAsyncCompute();

	auto node6 = std::make_unique<RenderNode>(*renderGraph, "n6_graphics");
    node6->CreateTexture("g3");
    node6->AsGenaralSampledImage("c2");
    node6->AsGenaralSampledImage("g2");

    renderGraph->AppendNode(std::move(node0));
    renderGraph->AppendNode(std::move(node1));
    renderGraph->AppendNode(std::move(node2));
    renderGraph->AppendNode(std::move(node3));
    renderGraph->AppendNode(std::move(node4));
    renderGraph->AppendNode(std::move(node5));
    renderGraph->AppendNode(std::move(node6));
    renderGraph->Compile();
}

void Renderer::Shutdown()
{
    spdlog::info("Shutdown Renderer.");
    renderPass.reset();
    depthStencilView.reset();
    depthStencil.reset();
    basicPipeline.reset();
    triFrag.reset();
    triVert.reset();
}
} // namespace sy::render
