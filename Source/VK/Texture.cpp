#include <Core.h>
#include <VK/Texture.h>
#include <VK/VulkanContext.h>

namespace sy
{
	Texture::Texture(const std::string_view name, const VulkanContext& vulkanContext) :
		VulkanWrapper(name, vulkanContext, VK_DESTROY_LAMBDA_SIGNATURE(VkImage)
		{
		})
	{
	}

	Texture::~Texture()
	{
		if (view != VK_NULL_HANDLE)
		{
			vkDestroyImageView(vulkanContext.GetDevice(), view, nullptr);
		}
		if (sampler != VK_NULL_HANDLE)
		{
			vkDestroySampler(vulkanContext.GetDevice(), sampler, nullptr);
		}

		if (handle != VK_NULL_HANDLE)
		{
			vmaDestroyImage(vulkanContext.GetAllocator(), handle, allocation);
		}
	}

	Texture2D::Texture2D(std::string_view name, const VulkanContext& vulkanContext, const Extent2D<uint32_t> extent, const uint32_t mipLevels, const VkFormat format,
		const VkImageUsageFlags usageFlags, const VkImageLayout layout) :
		Texture(name, vulkanContext)
	{
		const auto allocator = vulkanContext.GetAllocator();
		const VkImageCreateInfo imageCreateInfo
		{
			.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.imageType = VK_IMAGE_TYPE_2D,
			.format = format,
			.extent = VkExtent3D{extent.width, extent.height, 1},
			.mipLevels = mipLevels,
			.arrayLayers = 1,
			.samples = VK_SAMPLE_COUNT_1_BIT,
			.tiling = VK_IMAGE_TILING_OPTIMAL,
			.usage = usageFlags,
			.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
			.initialLayout = layout
		};

		const VmaAllocationCreateInfo allocationCreateInfo
		{
			.usage = VMA_MEMORY_USAGE_GPU_ONLY
		};

		VK_ASSERT(vmaCreateImage(allocator, &imageCreateInfo, &allocationCreateInfo, &handle, &allocation, nullptr), "Failed to create image {}.", name);

		const VkImageViewCreateInfo viewCreateInfo
		{
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.image = handle,
			.viewType = VK_IMAGE_VIEW_TYPE_2D,
			.format = format,
			.subresourceRange = VkImageSubresourceRange
			{
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.baseMipLevel = 0,
				.levelCount = mipLevels,
				.baseArrayLayer = 0,
				.layerCount = 1
			}
		};

		VK_ASSERT(vkCreateImageView(vulkanContext.GetDevice(), &viewCreateInfo, nullptr, &view), "Failed to create image view {}.", name);
	}
}
