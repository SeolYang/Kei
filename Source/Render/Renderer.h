#pragma once
#include <PCH.h>

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
		class FrameTracker;
		class CommandPoolManager;
		class DescriptorManager;
	}

	class Window;
	namespace render
	{
		class Mesh;
		class SimpleRenderPass;
		class Renderer final : public NonCopyable
		{
		public:
			Renderer(const Window& window, vk::VulkanContext& vulkanContext, const vk::FrameTracker& frameTracker, vk::CommandPoolManager& cmdPoolManager, vk::DescriptorManager& descriptorManager);
			~Renderer();

			void Render();

		private:
			void BeginFrame();
			void EndFrame();

		private:
			const Window& window;
			vk::VulkanContext& vulkanContext;
			const vk::FrameTracker& frameTracker;
			vk::CommandPoolManager& cmdPoolManager;
			vk::DescriptorManager& descriptorManager;

			std::unique_ptr<vk::PipelineLayoutCache> pipelineLayoutCache;

			std::unique_ptr<vk::ShaderModule> triVert;
			std::unique_ptr<vk::ShaderModule> triFrag;
			std::unique_ptr<vk::Pipeline> basicPipeline;

			std::unique_ptr<vk::Texture> depthStencil;
			std::unique_ptr<vk::TextureView> depthStencilView;

			std::unique_ptr<vk::Sampler> linearSampler;
			std::unique_ptr<vk::Texture> loadedTexture;
			std::unique_ptr<vk::TextureView> loadedTextureView;
			vk::Descriptor loadedTextureDescriptor;

			std::unique_ptr<Mesh> cube;

			std::unique_ptr<SimpleRenderPass> renderPass;

			glm::mat4 viewProjMat;
			float elapsedTime;

		};
	}
}