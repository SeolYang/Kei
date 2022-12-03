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
		static std::unique_ptr<Buffer> CreateBuffer(std::string_view name, const VulkanContext& vulkanContext, VkBufferCreateFlags bufferCreateFlags, VkBufferUsageFlags bufferUsageFlags, VmaMemoryUsage memoryUsage)
		{
			return std::make_unique<Buffer>(name, vulkanContext, sizeof(BufferData), bufferCreateFlags, bufferUsageFlags, memoryUsage);
		}


	private:
		VmaAllocation allocation;
		size_t bufferSize;
		const VkBufferUsageFlags bufferUsageFlags;

	};
}