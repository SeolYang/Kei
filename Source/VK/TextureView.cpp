#include <Core/Core.h>
#include <VK/TextureView.h>

#include "Texture.h"
#include "VulkanContext.h"

namespace sy::vk
{
	TextureView::TextureView(std::string_view name, const VulkanContext& vulkanContext, const Texture& texture, const VkImageViewType type,
		const VkFormat format, const VkImageAspectFlags aspects, const uint32_t baseMipLevel, const uint32_t mipLevelCount,
		const uint32_t baseArrayLayer, const uint32_t layerCount) :
		VulkanWrapper<VkImageView>(name, vulkanContext, VK_OBJECT_TYPE_IMAGE_VIEW, VK_DESTROY_LAMBDA_SIGNATURE(VkImageView)
		{
			vkDestroyImageView(vulkanContext.GetDevice(), handle, nullptr);
		}),
		texture(texture),
		viewType(type),
		format(format),
		aspects(aspects),
		baseMipLevel(baseMipLevel),
		mipLevelCount(mipLevelCount),
		baseArrayLayer(baseArrayLayer),
		layerCount(layerCount)
	{
		const VkImageViewCreateInfo viewCreateInfo
		{
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.image = texture.GetNativeHandle(),
			.viewType = type,
			.format = format,
			.subresourceRange = VkImageSubresourceRange
			{
				.aspectMask = aspects,
				.baseMipLevel = baseMipLevel,
				.levelCount = mipLevelCount,
				.baseArrayLayer = baseArrayLayer,
				.layerCount = layerCount
			}
		};
		VK_ASSERT(vkCreateImageView(vulkanContext.GetDevice(), &viewCreateInfo, nullptr, &handle), "Failed to create image view {}.", name);
	}

	TextureView::TextureView(std::string_view name, const VulkanContext& vulkanContext, const Texture& texture, const VkImageViewType type, const uint32_t baseMipLevel, const uint32_t mipLevelCount, const uint32_t baseArrayLayer, const uint32_t layerCount) :
		TextureView(name, vulkanContext, texture, type, texture.GetFormat(), IsDepthStencilFormat(texture.GetFormat()) ? VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT : VK_IMAGE_ASPECT_COLOR_BIT,
			baseMipLevel, mipLevelCount, baseArrayLayer, layerCount)
	{
	}
}
