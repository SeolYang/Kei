#pragma once
#include <Core.h>

namespace sy
{
	class VulkanContext;
	class Buffer : public VulkanWrapper<VkBuffer>
	{
	public:
		Buffer(std::string_view name, const VulkanContext& vulkanContext, size_t bufferSize, VkBufferCreateFlags bufferCreateFlags, VkBufferUsageFlags bufferUsageFlags, VmaMemoryUsage memoryUsage);
		virtual ~Buffer() override;

		Buffer(const Buffer&) = delete;
		Buffer(Buffer&&) = delete;

		Buffer& operator=(const Buffer&) = delete;
		Buffer& operator=(Buffer&&) = delete;

		[[nodiscard]] auto GetBufferSize() const { return bufferSize; }
		[[nodiscard]] auto GetBufferUsage() const { return bufferUsageFlags; }
		[[nodiscard]] VkDescriptorBufferInfo GetDescriptorInfo() const { return { handle, 0, bufferSize }; }
		[[nodiscard]] auto GetAllocation() const { return allocation; }

		template <typename BufferData>
		static std::unique_ptr<Buffer> CreateUniformBuffer(std::string_view name, const VulkanContext& vulkanContext, const VkBufferCreateFlags bufferCreateFlags = 0)
		{
			return std::make_unique<Buffer>(name, vulkanContext, sizeof(BufferData), bufferCreateFlags, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
		}


	private:
		VmaAllocation allocation;
		size_t bufferSize;
		const VkBufferUsageFlags bufferUsageFlags;

	};
}