#include <PCH.h>
#include <VK/TextureView.h>
#include <VK/Texture.h>
#include <VK/VulkanRHI.h>

namespace sy::vk
{
	TextureView::TextureView(const std::string_view name, const VulkanRHI& vulkanRHI, const Texture& texture,
		const VkImageViewType viewType, const TextureSubResourceRange subResourceRange)
		: VulkanWrapper<VkImageView>(name, vulkanRHI, VK_OBJECT_TYPE_IMAGE_VIEW)
		, viewType(viewType)
		, subResourceRange(subResourceRange)
	{
		/** @todo should custom format for texture view? */
		const auto format = texture.GetFormat();
		const VkImageViewCreateInfo viewCreateInfo{
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.image = texture.GetNative(),
			.viewType = viewType,
			.format = format,
			.subresourceRange = VkImageSubresourceRange{
				.aspectMask = FormatToImageAspect(format),
				.baseMipLevel = subResourceRange.MipLevel,
				.levelCount = subResourceRange.MipLevelCount,
				.baseArrayLayer = subResourceRange.ArrayLayer,
				.layerCount = subResourceRange.ArrayLayerCount }
		};

		NativeHandle handle = VK_NULL_HANDLE;
		VK_ASSERT(vkCreateImageView(vulkanRHI.GetDevice(), &viewCreateInfo, nullptr, &handle),
			"Failed to create image view {}.", name);

		UpdateHandle(
			handle, SY_VK_WRAPPER_DELETER(rhi) {
				vkDestroyImageView(rhi.GetDevice(), handle, nullptr);
			});
	}

	TextureView::TextureView(const std::string_view name, const VulkanRHI& vulkanRHI, const Texture& texture,
		const VkImageViewType viewType)
		: TextureView(name, vulkanRHI, texture, viewType, texture.GetFullSubResourceRange())
	{
	}
} // namespace sy::vk
