#pragma once
#include <Core.h>

namespace sy
{
	class VulkanContext;
	class CommandPoolManager;
	class FrameTracker;
	class Texture : public VulkanWrapper<VkImage>
	{
	public:
		Texture(std::string_view name, const VulkanContext& vulkanContext, Extent3D<uint32_t> extent, VkFormat format, VkImageLayout desiredImageLayout, VkImageUsageFlags imageUsageFlags, VmaMemoryUsage memoryUsage, uint32_t mipLevels = 1);
		virtual ~Texture() override;

		[[nodiscard]] auto GetExtent() const { return extent; }
		[[nodiscard]] VkDescriptorImageInfo GetDescriptorInfo() const { return { sampler, view, desiredImageLayout }; }
		[[nodiscard]] auto GetDesiredLayout() const { return desiredImageLayout; }
		[[nodiscard]] auto GetImageView() const { return view; }
		[[nodiscard]] auto GetMipLevels() const { return mipLevels; }
		[[nodiscard]] auto GetSampler() const { return sampler; }
		[[nodiscard]] auto GetFormat() const { return format; }
		[[nodiscard]] auto GetMemoryUsage() const { return memoryUsage; }
		[[nodiscard]] auto GetUsage() const { return imageUsage; }

		Texture(const Texture&) = delete;
		Texture(Texture&&) = delete;

		Texture& operator=(const Texture&) = delete;
		Texture& operator=(Texture&&) = delete;

	protected:
		VmaAllocation allocation = VK_NULL_HANDLE;
		VkImageView view = VK_NULL_HANDLE;
		VkSampler sampler = VK_NULL_HANDLE;

	private:
		Extent3D<uint32_t> extent;
		const VkFormat format;
		const uint32_t mipLevels;

		const VkImageLayout desiredImageLayout;
		const VkImageUsageFlags imageUsage;
		const VmaMemoryUsage memoryUsage;

	};

	class Texture2D : public Texture
	{
	public:
		Texture2D(std::string_view name, const VulkanContext& vulkanContext, Extent2D<uint32_t> extent, uint32_t mipLevels, VkFormat format, VkImageUsageFlags usageFlags, VkImageLayout initialLayout , VmaMemoryUsage memoryUsage);
		static std::unique_ptr<Texture2D> LoadFromFile(CommandPoolManager& cmdPoolManager, const FrameTracker& frameTracker, std::string_view filePath, const VulkanContext& vulkanContext, VkFormat format, VkImageUsageFlags usageFlags = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, VkImageLayout layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	};
}