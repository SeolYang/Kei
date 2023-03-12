#include <PCH.h>
#include <VK/VulkanRHI.h>
#include <VK/Fence.h>
#include <VK/Texture.h>
#include <VK/Buffer.h>
#include <VK/CommandPool.h>
#include <VK/CommandBuffer.h>
#include <VK/CommandPoolManager.h>
#include <VK/FrameTracker.h>

#include "VulkanContext.h"

namespace sy
{
	namespace vk
	{
		Texture::Texture(std::string_view name, const VulkanRHI& vulkanRHI, const TextureInfo& info, const bool bReserveMips, const ETextureState initialState) : VulkanWrapper(name, vulkanRHI, VK_OBJECT_TYPE_IMAGE, 
			VK_DESTROY_LAMBDA_SIGNATURE(VkImage)
		{
		}),
			info(info),
			initialState(initialState)
		{
			const bool bIsLegalToReserveMips = (info.Extent.width == info.Extent.height && info.Extent.height == info.Extent.depth) && (math::IsPowOfTwo(info.Extent.width));
			if (bIsLegalToReserveMips)
			{
				this->info.MipLevels = static_cast<uint32_t>(std::log2(info.Extent.width));
			}

			const auto allocator = vulkanRHI.GetAllocator();
			const auto extent = GetExtent();
			const VkImageCreateInfo imageCreateInfo
			{
				.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
				.pNext = nullptr,
				.flags = 0,
				.imageType = GetImageType(),
				.format = GetFormat(),
				.extent = VkExtent3D{extent.width, extent.height, extent.depth},
				.mipLevels = GetMipLevels(),
				.arrayLayers = GetArrayLayers(),
				.samples = GetSamples(),
				.tiling = GetTiling(),
				.usage = GetUsageFlags(),
				.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
				.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
			};

			const VmaAllocationCreateInfo allocationCreateInfo
			{
				.usage = GetMemoryUsage(),
				.requiredFlags = GetMemoryPropertyFlags()
			};

			Native_t handle = VK_NULL_HANDLE;
			VK_ASSERT(vmaCreateImage(allocator, &imageCreateInfo, &allocationCreateInfo, &handle, &allocation, nullptr), "Failed to create image {}.", name);
			UpdateHandle(handle);
		}

		Texture::~Texture()
		{
			if (allocation != VK_NULL_HANDLE)
			{
				const auto& vulkanRHI = GetContext();
				const auto handle = GetNativeHandle();
				vmaDestroyImage(vulkanRHI.GetAllocator(), handle, allocation);
			}
		}

		std::unique_ptr<Texture> CreateTexture2D(const std::string_view name, const VulkanRHI& vulkanRHI, TextureInfo info, const bool bReserveMips, const ETextureState initialState)
		{
			info.Type = VK_IMAGE_TYPE_2D;
			info.Extent.depth = 1;
			info.ArrayLayers = 1;

			return std::make_unique<Texture>(
				name,
				vulkanRHI,
				info,
				bReserveMips, initialState);
		}

		std::unique_ptr<Texture> CreateShaderResourceTexture2D(const std::string_view name, const VulkanContext& vulkanContext, TextureInfo info, const bool bReserveMips,
			const std::span<const char> textureData)
		{
			const auto& vulkanRHI = vulkanContext.GetVulkanRHI();
			auto& cmdPoolManager = vulkanContext.GetCommandPoolManager();
			constexpr auto InitialState = ETextureState::AnyShaderReadSampledImage;

			auto stagingBufferBuilder = BufferBuilder{ vulkanContext };
			stagingBufferBuilder.SetName(std::format("{}_Staging", name))
				.SetUsage(VK_BUFFER_USAGE_TRANSFER_SRC_BIT)
				.SetMemoryUsage(VMA_MEMORY_USAGE_CPU_ONLY)
				.SetSize(textureData.size());
			const auto stagingBuffer = stagingBufferBuilder.Build();
			void* mappedStagingBuffer = vulkanRHI.Map(*stagingBuffer);
			memcpy(mappedStagingBuffer, textureData.data(), textureData.size());
			vulkanRHI.Unmap(*stagingBuffer);

			info.UsageFlags = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
			info.MemoryPropertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
			info.MemoryUsage = VMA_MEMORY_USAGE_GPU_ONLY;
			auto newTexture = CreateTexture2D(name, vulkanRHI, info, bReserveMips, InitialState);

			auto& cmdPool = cmdPoolManager.RequestCommandPool(EQueueType::Graphics);
			const auto cmdBuffer = cmdPool.RequestCommandBuffer(std::format("{}_Upload Command Buffer", name));
			cmdBuffer->Begin();
			{
				cmdBuffer->ChangeState(vk::ETextureState::None, ETextureState::TransferWrite, *newTexture);
				cmdBuffer->CopyBufferToImageSimple(*stagingBuffer, *newTexture);
				cmdBuffer->ChangeState(vk::ETextureState::TransferWrite, InitialState, *newTexture);
			}
			cmdBuffer->End();

			vulkanRHI.SubmitImmediateTo(*cmdBuffer);
			return newTexture;
		}

		std::unique_ptr<Texture> LoadShaderResourceTexture2DFromFile(const std::string_view filePath, const std::string_view name,
			const VulkanContext& vulkanContext, const VkFormat format, const bool bReserveMips)
		{
			const auto& vulkanRHI = vulkanContext.GetVulkanRHI();
			auto& cmdPoolManager = vulkanContext.GetCommandPoolManager();

			int texWidth, texHeight;
			int texChannels;
			stbi_uc* pixels = stbi_load(filePath.data(), &texWidth, &texHeight, &texChannels, static_cast<int>(ToNumberOfComponents(format)));
			if (pixels == nullptr)
			{
				spdlog::warn("Failed to load image from file {}.", filePath);
				return nullptr;
			}

			const char* pixelsData = reinterpret_cast<char*>(pixels);
			const VkDeviceSize sizeOfData = (static_cast<uint64_t>(texWidth) * static_cast<uint64_t>(texHeight)) * ToByteSize(format);

			const TextureInfo info
			{
				.Extent = {static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight), 1},
				.Format = format
			};

			auto result = CreateShaderResourceTexture2D(
				name,
				vulkanContext,
				info, bReserveMips,
				std::span{ pixelsData, sizeOfData });

			stbi_image_free(pixels);
			return result;
		}

		std::unique_ptr<Texture> CreateDepthStencil(const std::string_view name, const VulkanContext& vulkanContext, const Extent2D<uint32_t> extent, const VkFormat format)
		{
			const auto& vulkanRHI = vulkanContext.GetVulkanRHI();
			auto& cmdPoolManager = vulkanContext.GetCommandPoolManager();
			const TextureInfo info
			{
				.Extent = {extent.width, extent.height, 1},
				.UsageFlags = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
				.Format = IsDepthStencilFormat(format) ? format : VK_FORMAT_D24_UNORM_S8_UINT,
				.MemoryPropertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				.MemoryUsage = VMA_MEMORY_USAGE_GPU_ONLY
			};

			auto newDepthStencil = CreateTexture2D(
				name, 
				vulkanRHI, 
				info, false, 
				ETextureState::DepthStencilAttachmentWrite);

			auto& cmdPool = cmdPoolManager.RequestCommandPool(EQueueType::Graphics);
			const auto cmdBuffer = cmdPool.RequestCommandBuffer(std::format("{}_State Transition Command Buffer", name));
			cmdBuffer->Begin();
			{
				cmdBuffer->ChangeState(vk::ETextureState::None, vk::ETextureState::DepthStencilAttachmentWrite, *newDepthStencil);
			}
			cmdBuffer->End();

			vulkanRHI.SubmitImmediateTo(*cmdBuffer);
			return newDepthStencil;
		}
	}
}
