#pragma once
#include <PCH.h>

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
			Buffer(std::string_view name, const VulkanContext& vulkanContext, BufferInfo info, EBufferState initialState = EBufferState::None);
			virtual ~Buffer() override;

			[[nodiscard]] auto GetInfo() const { return info; }
			[[nodiscard]] auto GetSize() const { return info.Size; }
			[[nodiscard]] auto GetUsageFlags() const { return info.UsageFlags; }
			[[nodiscard]] auto GetMemoryUsage() const { return info.MemoryUsage; }
			[[nodiscard]] VkDescriptorBufferInfo GetDescriptorInfo() const { return { GetNativeHandle(), 0, GetSize()}; }
			[[nodiscard]] const VmaAllocation& GetAllocation() const { return allocation; }
			[[nodiscard]] auto GetInitialState() const { return initialState; }

		private:
			BufferInfo info;
			EBufferState initialState;
			VmaAllocation allocation;

		};

		std::unique_ptr<Buffer> CreateBufferWithData(std::string_view name, const VulkanContext& vulkanContext, CommandPoolManager& cmdPoolManager, const FrameTracker& frameTracker, VkBufferUsageFlags bufferUsage, size_t sizeOfData, const void* data, EBufferState initialState = EBufferState::None);

		std::unique_ptr<Buffer> CreateStagingBuffer(std::string_view name, const VulkanContext& vulkanContext, size_t bufferSize);

		template <typename BufferData>
		auto CreateUniformBuffer(std::string_view name, const VulkanContext& vulkanContext)
		{
			return std::make_unique<Buffer>(name, vulkanContext, BufferInfo{ sizeof(BufferData), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU }, EBufferState::AnyShaderReadUniformBuffer);
		}

		template <typename Vertex>
		auto CreateVertexBuffer(const std::string_view name, const VulkanContext& vulkanContext, CommandPoolManager& cmdPoolManager, const FrameTracker& frameTracker, const std::span<Vertex> vertices)
		{
			/** @todo initial state */
			return CreateBufferWithData(name, vulkanContext, cmdPoolManager, frameTracker, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, sizeof(Vertex) * vertices.size(), vertices.data(), EBufferState::VertexBuffer);
		}

		template <typename Index = uint32_t>
		std::unique_ptr<Buffer> CreateIndexBuffer(const std::string_view name, const VulkanContext& vulkanContext, CommandPoolManager& cmdPoolManager, const FrameTracker& frameTracker, std::span<const Index> indices)
		{
			/** @todo initial state */
			return CreateBufferWithData(name, vulkanContext, cmdPoolManager, frameTracker, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, sizeof(Index) * indices.size(), indices.data(), EBufferState::IndexBuffer);
		}
	}
}