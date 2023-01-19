#include <PCH.h>
#include <VK/TextureView.h>
#include <VK/Texture.h>
#include <VK/VulkanContext.h>

namespace sy::vk
{
	TextureView::TextureView(const std::string_view name, const VulkanContext& vulkanContext, const Texture& texture,
		const VkImageViewType viewType, const TextureSubResource subResource) :
		VulkanWrapper<VkImageView>(name, vulkanContext, VK_OBJECT_TYPE_IMAGE_VIEW, VK_DESTROY_LAMBDA_SIGNATURE(VkImageView)
	{
		vkDestroyImageView(vulkanContext.GetDevice(), handle, nullptr);
	}),
		viewType(viewType),
		subResource(subResource)
	{
		const VkImageViewCreateInfo viewCreateInfo
		{
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.image = texture.GetNativeHandle(),
			.viewType = viewType,
			.format = subResource.Format,
			.subresourceRange = VkImageSubresourceRange
			{
				.aspectMask = FormatToImageAspect(subResource.Format),
				.baseMipLevel = subResource.MipLevel,
				.levelCount = subResource.MipLevelCount,
				.baseArrayLayer = subResource.ArrayLayer,
				.layerCount = subResource.ArrayLayerCount
			}
		};

		Native_t handle = VK_NULL_HANDLE;
		VK_ASSERT(vkCreateImageView(vulkanContext.GetDevice(), &viewCreateInfo, nullptr, &handle), "Failed to create image view {}.", name);
		UpdateHandle(handle);
	}

	TextureView::TextureView(const std::string_view name, const VulkanContext& vulkanContext, const Texture& texture,
		const VkImageViewType viewType) :
		TextureView(name, vulkanContext, texture, viewType, texture.GetFullSubResource())
	{
	}
}
