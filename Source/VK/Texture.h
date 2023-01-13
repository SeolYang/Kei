#pragma once
#include <PCH.h>

namespace sy
{
	namespace vk
	{
		class VulkanContext;
		class CommandPoolManager;
		class FrameTracker;
		class Texture : public VulkanWrapper<VkImage>
		{
		public:
			Texture(std::string_view name, const VulkanContext& vulkanContext, VkImageType type, VkFormat format, Extent3D<uint32_t> extent, uint32_t mipLevels, uint32_t arrayLayers, VkSampleCountFlags samples, VkImageTiling tiling, VkImageUsageFlags usage, VmaMemoryUsage memoryUsage, VkMemoryPropertyFlags memoryProperties);
			virtual ~Texture() override;

			[[nodiscard]] auto GetExtent() const { return extent; }
			[[nodiscard]] auto GetMipLevels() const { return mipLevels; }
			[[nodiscard]] auto GetFormat() const { return format; }
			[[nodiscard]] auto GetMemoryUsage() const { return memoryUsage; }
			[[nodiscard]] auto GetUsage() const { return imageUsage; }
			[[nodiscard]] auto GetMemoryProperties() const { return memoryProperties; }

			Texture(const Texture&) = delete;
			Texture(Texture&&) = delete;

			Texture& operator=(const Texture&) = delete;
			Texture& operator=(Texture&&) = delete;

		protected:
			VmaAllocation allocation = VK_NULL_HANDLE;

		private:
			Extent3D<uint32_t> extent;
			const VkFormat format;
			const uint32_t mipLevels;

			const VkImageUsageFlags imageUsage;
			const VmaMemoryUsage memoryUsage;
			const VkMemoryPropertyFlags memoryProperties;

		};

		std::unique_ptr<Texture> CreateTexture2D(std::string_view name, const VulkanContext& vulkanContext, Extent2D<uint32_t> extent, uint32_t mipLevels, VkFormat format, VkImageUsageFlags usages, VmaMemoryUsage memoryUsage, VkMemoryPropertyFlags memoryProperties = 0);
		std::unique_ptr<Texture> CreateTexture2DFromMemory(std::string_view name, const VulkanContext& vulkanContext, const FrameTracker& frameTracker, CommandPoolManager& cmdPoolManager, std::span<const char> data, Extent2D<uint32_t> extent, VkFormat format, VkImageUsageFlags usageFlags = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, VkImageLayout layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		std::unique_ptr<Texture> CreateTexture2DFromFile(std::string_view filePath, const VulkanContext& vulkanContext, const FrameTracker& frameTracker, CommandPoolManager& cmdPoolManager, VkFormat format);
		std::unique_ptr<Texture> CreateDepthStencil(std::string_view name, const VulkanContext& vulkanContext, const FrameTracker& frameTracker, CommandPoolManager& cmdPoolManager, Extent2D<uint32_t> extent);

	}
}
