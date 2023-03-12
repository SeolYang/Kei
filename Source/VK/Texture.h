#pragma once
#include <PCH.h>

namespace sy::vk
{
	constexpr std::string_view DefaultWhiteTexture = "Default_White_Tex";
	constexpr std::string_view DefaultBlackTexture = "Default_Black_Tex";

	class VulkanContext;
	class CommandPoolManager;
	class FrameTracker;
	class TextureBuilder;
	class Texture : public VulkanWrapper<VkImage>
	{
	public:
		explicit Texture(const TextureBuilder& builder);
		~Texture() override;

		[[nodiscard]] auto GetExtent() const { return extent; }
		[[nodiscard]] auto GetMipLevels() const { return mips; }
		[[nodiscard]] auto GetArrayLayers() const { return layers; }
		[[nodiscard]] auto GetFormat() const { return format; }
		[[nodiscard]] auto GetMemoryUsage() const { return memoryUsage; }
		[[nodiscard]] auto GetUsageFlags() const { return usage; }
		[[nodiscard]] auto GetMemoryPropertyFlags() const { return memoryProperty; }
		[[nodiscard]] auto GetImageType() const { return type; }
		[[nodiscard]] auto GetSamples() const { return samples; }
		[[nodiscard]] auto GetTiling() const { return tiling; }
		/** @warning Assume Texture Array = 2D Textures array */
		[[nodiscard]] auto IsTextureArray() const { return GetImageType() == VK_IMAGE_TYPE_2D && GetExtent().depth > 1; }
		[[nodiscard]] auto GetNumSubResources() const { return IsTextureArray() ? GetExtent().depth * GetMipLevels() : GetMipLevels(); }
		[[nodiscard]] auto GetInitialState() const { return initialState; }

		[[nodiscard]] TextureSubResourceRange GetFullSubResourceRange() const { return { 0, GetMipLevels(), 0, GetArrayLayers() }; }

	private:
		VmaAllocation allocation = VK_NULL_HANDLE;
		const VkImageType type;
		const VkImageUsageFlags usage;
		const VkFormat format;
		const VmaMemoryUsage memoryUsage;
		const VkMemoryPropertyFlags memoryProperty;
		const Extent3D<uint32_t> extent;
		const uint32_t layers;
		const VkSampleCountFlagBits samples;
		const VkImageTiling tiling;
		const ETextureState initialState;
		const bool bGenerateMips;
		const uint32_t mips;

	};
}
