#pragma once
#include <Core/Core.h>

namespace sy
{
	namespace vk
	{
		class VulkanContext;
		class Semaphore;
		class ShaderModule;
		class Pipeline;
		class Buffer;
		class Fence;
		class Texture2D;
		class PipelineLayoutCache;
		class FrameTracker;
		class CommandPoolManager;
		class DescriptorManager;
	}

	class Window;
	namespace render
	{
		class Renderer
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

			std::unique_ptr<vk::Texture2D> depthStencil;

			std::array<std::unique_ptr<vk::Buffer>, vk::NumMaxInFlightFrames> transformBuffers;
			std::array<OffsetSlotPtr, vk::NumMaxInFlightFrames> transformBufferIndices;

			std::unique_ptr<vk::Texture2D> loadedTexture;
			OffsetSlotPtr loadedTextureDescriptor;

			std::unique_ptr<vk::Buffer> cubeVertexBuffer;
			std::unique_ptr<vk::Buffer> cubeIndexBuffer;

			glm::mat4 viewProjMat;
			float elapsedTime;

		};
	}
}