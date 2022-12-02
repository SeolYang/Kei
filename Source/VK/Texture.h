#pragma once
#include <Core.h>

namespace sy
{
	class VulkanContext;
	class Texture : public VulkanWrapper<VkImage>
	{
	public:
		Texture(std::string_view name, const VulkanContext& vulkanContext, VkFormat format, VkImageLayout initialLayout, VmaMemoryUsage memoryUsage, uint32_t mipLevels = 1);
		virtual ~Texture() override;

		[[nodiscard]] auto GetExtent() const { return extent; }
		[[nodiscard]] VkDescriptorImageInfo GetDescriptorImageInfo() const { return { sampler, view, initialLayout }; }
		[[nodiscard]] auto GetInitialImageLayout() const { return initialLayout; }
		[[nodiscard]] auto GetImageView() const { return view; }
		[[nodiscard]] auto GetMipLevels() const { return mipLevels; }
		[[nodiscard]] auto GetSampler() const { return sampler; }
		[[nodiscard]] auto GetFormat() const { return format; }
		[[nodiscard]] auto GetMemoryUsage() const { return memoryUsage; }

		Texture(const Texture&) = delete;
		Texture(Texture&&) = delete;

		Texture& operator=(const Texture&) = delete;
		Texture& operator=(Texture&&) = delete;

	protected:
		VmaAllocation allocation;
		VkImageView view = VK_NULL_HANDLE;
		VkSampler sampler = VK_NULL_HANDLE;

		Extent3D<uint32_t> extent;
		const VkFormat format;
		const uint32_t mipLevels = 1;

		const VkImageLayout initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		const VmaMemoryUsage memoryUsage;

	};

	class Texture2D : public Texture
	{
	public:
		Texture2D(std::string_view name, const VulkanContext& vulkanContext, Extent2D<uint32_t> extent, uint32_t mipLevels, VkFormat format, VkImageUsageFlags usageFlags, VkImageLayout initialLayout = VK_IMAGE_LAYOUT_UNDEFINED, VmaMemoryUsage memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY);
		static std::unique_ptr<Texture2D> LoadFromFile(std::string_view name, const VulkanContext& vulkanContext, VkFormat format, VkImageUsageFlags usageFlags = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, VkImageLayout layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	};
}