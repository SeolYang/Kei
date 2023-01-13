#pragma once
#include <PCH.h>

namespace sy::vk
{
	class VulkanContext;
	class Texture;
	class TextureView : public VulkanWrapper<VkImageView>
	{
	public:
		TextureView(std::string_view name, const VulkanContext& vulkanContext, const Texture& texture, VkImageViewType type, VkFormat format, VkImageAspectFlags aspects, uint32_t baseMipLevel = 0, uint32_t mipLevelCount = 1, uint32_t baseArrayLayer = 0, uint32_t layerCount = 1);
		TextureView(std::string_view name, const VulkanContext& vulkanContext, const Texture& texture, VkImageViewType type, uint32_t baseMipLevel = 0, uint32_t mipLevelCount = 1, uint32_t baseArrayLayer = 0, uint32_t layerCount = 1);
		virtual ~TextureView() override = default;

	private:
		const Texture& texture;
		const VkImageViewType viewType;
		const VkFormat format;
		const VkImageAspectFlags aspects;
		const uint32_t baseMipLevel;
		const uint32_t mipLevelCount;
		const uint32_t baseArrayLayer;
		const uint32_t layerCount;

	};
}