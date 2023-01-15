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
			Texture(std::string_view name, const VulkanContext& vulkanContext, VkImageType type, VkFormat format, Extent3D<uint32_t> extent, uint32_t mipLevels, uint32_t arrayLayers, VkSampleCountFlagBits samples, VkImageTiling tiling, VkImageUsageFlags usage, VmaMemoryUsage memoryUsage, VkMemoryPropertyFlags memoryProperties, ETextureState initialState = ETextureState::None);
			~Texture() override;

			[[nodiscard]] auto GetExtent() const { return extent; }
			[[nodiscard]] auto GetMipLevels() const { return mipLevels; }
			[[nodiscard]] auto GetArrayLayers() const { return arrayLayers; }
			[[nodiscard]] auto GetFormat() const { return format; }
			[[nodiscard]] auto GetMemoryUsage() const { return memoryUsage; }
			[[nodiscard]] auto GetUsage() const { return imageUsage; }
			[[nodiscard]] auto GetMemoryProperties() const { return memoryProperties; }
			/** @warning Assume Texture Array = 2D Textures array */
			[[nodiscard]] auto IsTextureArray() const { return type == VK_IMAGE_TYPE_2D && extent.depth > 1; }
			[[nodiscard]] auto GetNumSubResources() const { return IsTextureArray() ? extent.depth * GetMipLevels() : GetMipLevels(); }
			[[nodiscard]] auto GetInitialState() const { return initialState; }

		protected:
			VmaAllocation allocation = VK_NULL_HANDLE;

		private:
			Extent3D<uint32_t> extent;
			const VkImageType type;
			const VkFormat format;
			const uint32_t mipLevels;
			const uint32_t arrayLayers;

			const VkImageUsageFlags imageUsage;
			const VmaMemoryUsage memoryUsage;
			const VkMemoryPropertyFlags memoryProperties;

			const ETextureState initialState;

		};

		std::unique_ptr<Texture> CreateTexture2D(std::string_view name, const VulkanContext& vulkanContext, Extent2D<uint32_t> extent, uint32_t mipLevels, VkFormat format, VkImageUsageFlags usages, VmaMemoryUsage memoryUsage, VkMemoryPropertyFlags memoryProperties = 0, ETextureState initialState = ETextureState::None);
		std::unique_ptr<Texture> CreateTexture2DFromMemory(std::string_view name, const VulkanContext& vulkanContext, const FrameTracker& frameTracker, CommandPoolManager& cmdPoolManager, std::span<const char> data, Extent2D<uint32_t> extent, VkFormat format);
		std::unique_ptr<Texture> CreateTexture2DFromFile(std::string_view filePath, const VulkanContext& vulkanContext, const FrameTracker& frameTracker, CommandPoolManager& cmdPoolManager, VkFormat format);
		std::unique_ptr<Texture> CreateDepthStencil(std::string_view name, const VulkanContext& vulkanContext, const FrameTracker& frameTracker, CommandPoolManager& cmdPoolManager, Extent2D<uint32_t> extent);

	}
}
