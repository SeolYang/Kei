#include <Core.h>
#include <VK/Buffer.h>

#include "VulkanContext.h"

namespace sy
{
	Buffer::Buffer(std::string_view name, const VulkanContext& vulkanContext, const size_t bufferSize, const VkBufferCreateFlags bufferCreateFlags, const VkBufferUsageFlags bufferUsageFlags,
		const VmaMemoryUsage memoryUsage) :
		VulkanWrapper(name, vulkanContext)
	{

		const VkBufferCreateInfo createInfo
		{
			.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
			.pNext = nullptr,
			.flags = bufferCreateFlags,
			.size = bufferSize,
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
