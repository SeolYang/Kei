#pragma once
#include <Core/Core.h>

namespace sy
{
	namespace vk
	{
		class VulkanContext;
		class CommandPoolManager;
		class FrameTracker;
		class Buffer : public VulkanWrapper<VkBuffer>
		{
		public:
			Buffer(std::string_view name, const VulkanContext& vulkanContext, size_t bufferSize, VkBufferCreateFlags bufferCreateFlags, VkBufferUsageFlags bufferUsageFlags, VmaMemoryUsage memoryUsage);
			virtual ~Buffer() override;

			[[nodiscard]] auto GetBufferSize() const { return bufferSize; }
			[[nodiscard]] auto GetUsage() const { return bufferUsageFlags; }
			[[nodiscard]] VkDescriptorBufferInfo GetDescriptorInfo() const { return { handle, 0, bufferSize }; }
			[[nodiscard]] auto GetAllocation() const { return allocation; }

		private:
			VmaAllocation allocation;
			size_t bufferSize;
			const VkBufferUsageFlags bufferUsageFlags;

		};

		template <typename BufferData>
		auto CreateUniformBuffer(std::string_view name, const VulkanContext& vulkanContext, const VkBufferCreateFlags bufferCreateFlags = 0)
		{
			return std::make_unique<Buffer>(name, vulkanContext, sizeof(BufferData), bufferCreateFlags, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
		}

		std::unique_ptr<Buffer> CreateStagingBuffer(std::string_view name, const VulkanContext& vulkanContext, const size_t bufferSize);

		template <typename Vertex>
		auto CreateVertexBuffer(CommandPoolManager& cmdPoolManager, const FrameTracker& frameTracker, std::string_view name, const VulkanContext& vulkanContext, const std::span<Vertex> vertices)
		{
			return CreateBufferWithData(cmdPoolManager, frameTracker, name, vulkanContext, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, sizeof(Vertex) * vertices.size(), vertices.data());
		}

		std::unique_ptr<Buffer> CreateIndexBuffer(CommandPoolManager& cmdPoolManager, const FrameTracker& frameTracker, std::string_view name, const VulkanContext& vulkanContext, std::span<const uint32_t> indices);

		std::unique_ptr<Buffer> CreateBufferWithData(CommandPoolManager& cmdPoolManager, const FrameTracker& frameTracker, std::string_view name, const VulkanContext& vulkanContext, VkBufferUsageFlags bufferUsage, size_t sizeOfData, const void* data);

	}
}