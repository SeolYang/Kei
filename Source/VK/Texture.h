#pragma once
#include <PCH.h>

namespace sy::vk
{
	constexpr std::string_view DefaultWhiteTexture = "Default_White_Tex";
	constexpr std::string_view DefaultBlackTexture = "Default_Black_Tex";

	class VulkanContext;
	class CommandPoolManager;
	class FrameTracker;
	class Texture : public VulkanWrapper<VkImage>
	{
	public:
		Texture(std::string_view name, const VulkanRHI& vulkanRHI, const TextureInfo& info, bool bReserveMips, ETextureState initialState);
		~Texture() override;

		[[nodiscard]] auto GetExtent() const { return info.Extent; }
		[[nodiscard]] auto GetMipLevels() const { return info.MipLevels; }
		[[nodiscard]] auto GetArrayLayers() const { return info.ArrayLayers; }
		[[nodiscard]] auto GetFormat() const { return info.Format; }
		[[nodiscard]] auto GetMemoryUsage() const { return info.MemoryUsage; }
		[[nodiscard]] auto GetUsageFlags() const { return info.UsageFlags; }
		[[nodiscard]] auto GetMemoryPropertyFlags() const { return info.MemoryPropertyFlags; }
		[[nodiscard]] auto GetImageType() const { return info.Type; }
		[[nodiscard]] auto GetSamples() const { return info.Samples; }
		[[nodiscard]] auto GetTiling() const { return info.Tiling; }
		/** @warning Assume Texture Array = 2D Textures array */
		[[nodiscard]] auto IsTextureArray() const { return GetImageType() == VK_IMAGE_TYPE_2D && GetExtent().depth > 1; }
		[[nodiscard]] auto GetNumSubResources() const { return IsTextureArray() ? GetExtent().depth * GetMipLevels() : GetMipLevels(); }
		[[nodiscard]] auto GetInitialState() const { return initialState; }

		[[nodiscard]] TextureSubResourceRange GetFullSubResourceRange() const { return { 0, GetMipLevels(), 0, GetArrayLayers() }; }

	private:
		TextureInfo info;
		const ETextureState initialState;
		VmaAllocation allocation = VK_NULL_HANDLE;

	};

	/**
	 * @ignored_params TextureInfo::Extent::depth, TextureInfo::Type, TextureInfo::ArrayLayers
	 */
	std::unique_ptr<Texture> CreateTexture2D(std::string_view name, const VulkanRHI& vulkanRHI, TextureInfo info, bool bReserveMips, ETextureState initialState);

	/**
	 * @ignored_params TextureInfo::Extent::depth, TextureInfo::Type, TextureInfo::UsageFlags, TextureInfo::MemoryPropertyFlags, TextureInfo::MemoryUsage, TextureInfo::ArrayLayers
	 */
	std::unique_ptr<Texture> CreateShaderResourceTexture2D(std::string_view name, const VulkanContext& vulkanContext, TextureInfo info, bool bReserveMips, std::span<const char> textureData);

	std::unique_ptr<Texture> LoadShaderResourceTexture2DFromFile(std::string_view filePath, std::string_view name, const VulkanContext& vulkanContext, CommandPoolManager& cmdPoolManager, VkFormat format, bool bReserveMips);
	std::unique_ptr<Texture> CreateDepthStencil(std::string_view name, const VulkanContext& vulkanContext, Extent2D<uint32_t> extent, const VkFormat format);
}
