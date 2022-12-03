#include <Core.h>
#include <VK/Buffer.h>

#include "VulkanContext.h"

namespace sy
{
	Buffer::Buffer(std::string_view name, const VulkanContext& vulkanContext, const size_t bufferSize, const VkBufferCreateFlags bufferCreateFlags, const VkBufferUsageFlags bufferUsageFlags,
		const VmaMemoryUsage memoryUsage) :
		VulkanWrapper(name, vulkanContext),
		bufferUsageFlags(bufferUsageFlags)
	{
		size_t alignedBufferSize = bufferSize;
		switch (bufferUsageFlags)
		{
		case VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT:
			alignedBufferSize = vulkanContext.PadUniformBufferSize(bufferSize);
			break;

		case VK_BUFFER_USAGE_STORAGE_BUFFER_BIT:
			alignedBufferSize = vulkanContext.PadStorageBufferSize(bufferSize);
			break;

		default:
			break;
		}
		this->bufferSize = alignedBufferSize;

		const VkBufferCreateInfo createInfo
		{
			.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
			.pNext = nullptr,
			.flags = bufferCreateFlags,
			.size = alignedBufferSize,
			.usage = bufferUsageFlags,
			.sharingMode = VK_SHARING_MODE_EXCLUSIVE
		};

		const VmaAllocationCreateInfo allocationCreateInfo
		{
			.usage = memoryUsage
		};

		VK_ASSERT(vmaCreateBuffer(vulkanContext.GetAllocator(), &createInfo, &allocationCreateInfo, &handle, &allocation, nullptr), "Failed to create buffer {}.", name);
	}

	Buffer::~Buffer()
	{
		vmaDestroyBuffer(vulkanContext.GetAllocator(), handle, allocation);
	}
}
