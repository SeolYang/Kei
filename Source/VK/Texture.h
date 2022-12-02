#pragma once
#include <Core.h>

namespace sy
{
	class VulkanContext;
	class Texture : public VulkanWrapper<VkImage>
	{
	public:
		Texture(std::string_view name, const VulkanContext& vulkanContext);
		virtual ~Texture() override;

		[[nodiscard]] auto GetExtent() const { return extent; }
		[[nodiscard]] VkDescriptorImageInfo GetDescriptorImageInfo() const { return { sampler, view, layout }; }
		[[nodiscard]] auto GetImageLayout() const { return layout; }
		[[nodiscard]] auto GetImageView() const { return view; }
		[[nodiscard]] auto GetMipLevels() const { return mipLevels; }
		[[nodiscard]] auto GetSampler() const { return sampler; }

		Texture(const Texture&) = delete;
		Texture(Texture&&) = delete;

		Texture& operator=(const Texture&) = delete;
		Texture& operator=(Texture&&) = delete;

	protected:
		/** Width, Height, Depth */
		VmaAllocation allocation;
		Extent3D<uint32_t> extent;
		VkImageLayout layout = VK_IMAGE_LAYOUT_UNDEFINED;
		VkImageView view = VK_NULL_HANDLE;
		uint32_t mipLevels = 1;
		VkSampler sampler = VK_NULL_HANDLE;

	};

	class Texture2D : public Texture
	{
	public:
		Texture2D(std::string_view name, const VulkanContext& vulkanContext, Extent2D<uint32_t> extent, uint32_t mipLevels, VkFormat format, VkImageUsageFlags usageFlags, VkImageLayout layout);
		static std::unique_ptr<Texture2D> LoadFromFile(std::string_view name, const VulkanContext& vulkanContext, VkFormat format, VkImageUsageFlags usageFlags = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, VkImageLayout layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	};
}