#pragma once
#include <PCH.h>
#include <Component/StaticMeshComponent.h>

namespace sy::vk
{
class VulkanContext;
class Semaphore;
class ShaderModule;
class Pipeline;
class Buffer;
class Texture;
class TextureView;
class Sampler;
class Fence;
class PipelineLayoutCache;
class ResourceStateTracker;
class FrameTracker;
class CommandPoolManager;
class DescriptorManager;
} // namespace sy::vk

namespace sy::window
{
class Window;
}

namespace sy::render
{
class Mesh;
class SimpleRenderPass;
/** @todo Renderer to RenderContext? */
class Renderer final : public Subsystem
{
public:
    Renderer(const window::Window& window, vk::VulkanContext& vulkanContext, HandleManager& handleManager);
    ~Renderer() override;

    void Startup();
    void Shutdown();

    void Render();

private:
    void BeginFrame();
    void EndFrame();

private:
    const window::Window& window;
    vk::VulkanContext&    vulkanContext;
    HandleManager&        handleManager;

    std::unique_ptr<vk::ResourceStateTracker> resStateTracker;

    std::unique_ptr<vk::ShaderModule> triVert;
    std::unique_ptr<vk::ShaderModule> triFrag;
    std::unique_ptr<vk::Pipeline>     basicPipeline;

    std::unique_ptr<vk::Texture>     depthStencil;
    std::unique_ptr<vk::TextureView> depthStencilView;

    std::unique_ptr<SimpleRenderPass> renderPass;

    glm::mat4 viewProjMat;
    float     elapsedTime;

    std::vector<component::StaticMeshComponent> staticMeshes;
};
} // namespace sy::render
