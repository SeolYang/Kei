#include <PCH.h>
#include <VK/TextureView.h>
#include <VK/Texture.h>
#include <VK/VulkanContext.h>

namespace sy::vk
{
	TextureView::TextureView(const std::string_view name, const VulkanContext& vulkanContext, const Texture& texture,
		const VkImageViewType viewType, const TextureSubResourceRange subResourceRange) :
		VulkanWrapper<VkImageView>(name, vulkanContext, VK_OBJECT_TYPE_IMAGE_VIEW, VK_DESTROY_LAMBDA_SIGNATURE(VkImageView)
	{
		vkDestroyImageView(vulkanContext.GetDevice(), handle, nullptr);
	}),
		viewType(viewType),
		subResourceRange(subResourceRange)
	{
		/** @todo should custom format for texture view? */
		const auto format = texture.GetFormat();
		const VkImageViewCreateInfo viewCreateInfo
		{
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.image = texture.GetNativeHandle(),
			.viewType = viewType,
			.format = format,
			.subresourceRange = VkImageSubresourceRange
			{
				.aspectMask = FormatToImageAspect(format),
				.baseMipLevel = subResourceRange.MipLevel,
				.levelCount = subResourceRange.MipLevelCount,
				.baseArrayLayer = subResourceRange.ArrayLayer,
				.layerCount = subResourceRange.ArrayLayerCount
			}
		};

		Native_t handle = VK_NULL_HANDLE;
		VK_ASSERT(vkCreateImageView(vulkanContext.GetDevice(), &viewCreateInfo, nullptr, &handle), "Failed to create image view {}.", name);
		UpdateHandle(handle);
	}

	TextureView::TextureView(const std::string_view name, const VulkanContext& vulkanContext, const Texture& texture,
		const VkImageViewType viewType) :
		TextureView(name, vulkanContext, texture, viewType, texture.GetFullSubResourceRange())
	{
	}
}
