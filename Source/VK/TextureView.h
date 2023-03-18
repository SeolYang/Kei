#pragma once
#include <PCH.h>

namespace sy::vk
{
	class VulkanRHI;
	class Texture;

	/** @deprecated TextureView will be encapsulated into texture. */
	class TextureView : public VulkanWrapper<VkImageView>
	{
	public:
		TextureView(std::string_view name, const VulkanRHI& vulkanRHI, const Texture& texture,
			VkImageViewType viewType, TextureSubResourceRange subResourceRange);
		TextureView(std::string_view name, const VulkanRHI& vulkanRHI, const Texture& texture,
			VkImageViewType viewType);

		~TextureView() override = default;

	private:
		const VkImageViewType viewType;
		const TextureSubResourceRange subResourceRange;
	};
} // namespace sy::vk
