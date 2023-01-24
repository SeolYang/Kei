#pragma once
#include <PCH.h>
#include <Component/StaticMeshComponent.h>

namespace sy
{
	namespace vk
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
	}

	class Window;
	class ResourceCache;
	namespace render
	{
		class Mesh;
		class SimpleRenderPass;
		class Renderer final : public NonCopyable
		{
		public:
			Renderer(const Window& window, vk::VulkanContext& vulkanContext, vk::ResourceStateTracker& resStateTracker, const vk::FrameTracker& frameTracker, vk::CommandPoolManager& cmdPoolManager, vk::DescriptorManager& descriptorManager, ResourceCache& resourceCache);
			~Renderer() override;

			void Render();

		private:
			void BeginFrame();
			void EndFrame();

		private:
			const Window& window;
			vk::VulkanContext& vulkanContext;
			vk::ResourceStateTracker& resStateTracker;
			const vk::FrameTracker& frameTracker;
			vk::CommandPoolManager& cmdPoolManager;
			vk::DescriptorManager& descriptorManager;
			ResourceCache& resourceCache;

			std::unique_ptr<vk::PipelineLayoutCache> pipelineLayoutCache;

			std::unique_ptr<vk::ShaderModule> triVert;
			std::unique_ptr<vk::ShaderModule> triFrag;
			std::unique_ptr<vk::Pipeline> basicPipeline;

			std::unique_ptr<vk::Texture> depthStencil;
			std::unique_ptr<vk::TextureView> depthStencilView;
			
			std::unique_ptr<SimpleRenderPass> renderPass;

			glm::mat4 viewProjMat;
			float elapsedTime;

			std::vector<component::StaticMeshComponent> staticMeshes;

		};
	}
}