#include <Core/Core.h>
#include <VK/VulkanContext.h>
#include <VK/Fence.h>
#include <VK/Texture.h>
#include <VK/Buffer.h>
#include <VK/CommandPool.h>
#include <VK/CommandBuffer.h>
#include <VK/CommandPoolManager.h>
#include <VK/FrameTracker.h>

namespace sy
{
	namespace vk
	{
		Texture::Texture(std::string_view name, const VulkanContext& vulkanContext, VkImageType type, VkFormat format, Extent3D<uint32_t> extent, uint32_t mipLevels, uint32_t arrayLayers, VkSampleCountFlags samples, VkImageTiling tiling, VkImageUsageFlags usage, VmaMemoryUsage memoryUsage, VkMemoryPropertyFlags memoryProperties) :
			VulkanWrapper(name, vulkanContext, VK_OBJECT_TYPE_IMAGE, VK_DESTROY_LAMBDA_SIGNATURE(VkImage)
			{
			}),
			extent(extent),
			format(format),
			mipLevels(mipLevels),
			imageUsage(usage),
			memoryUsage(memoryUsage),
			memoryProperties(memoryProperties)
		{
			const auto allocator = vulkanContext.GetAllocator();
			const VkImageCreateInfo imageCreateInfo
			{
				.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
				.pNext = nullptr,
				.flags = 0,
				.imageType = type,
				.format = format,
				.extent = VkExtent3D{extent.width, extent.height, 1},
				.mipLevels = mipLevels,
				.arrayLayers = 1,
				.samples = VK_SAMPLE_COUNT_1_BIT,
				.tiling = VK_IMAGE_TILING_OPTIMAL,
				.usage = usage,
				.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
				.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
			};

			const VmaAllocationCreateInfo allocationCreateInfo
			{
				.usage = memoryUsage,
				.requiredFlags = memoryProperties
			};
			VK_ASSERT(vmaCreateImage(allocator, &imageCreateInfo, &allocationCreateInfo, &handle, &allocation, nullptr), "Failed to create image {}.", name);
		}

		Texture::~Texture()
		{
			if (allocation != VK_NULL_HANDLE)
			{
				vmaDestroyImage(vulkanContext.GetAllocator(), handle, allocation);
			}
		}

		std::unique_ptr<Texture> CreateTexture2D(std::string_view name, const VulkanContext& vulkanContext,
			Extent2D<uint32_t> extent, uint32_t mipLevels, VkFormat format, VkImageUsageFlags usages,
			VmaMemoryUsage memoryUsage, VkMemoryPropertyFlags memoryProperties)
		{
			return std::make_unique<Texture>(name, vulkanContext, VK_IMAGE_TYPE_2D, format, Extent3D<uint32_t>{extent.width, extent.height, 1}, mipLevels, 1, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_TILING_OPTIMAL, usages, memoryUsage, memoryProperties);
		}

		std::unique_ptr<Texture> CreateTexture2DFromMemory(std::string_view name, const VulkanContext& vulkanContext,
			const FrameTracker& frameTracker, CommandPoolManager& cmdPoolManager,
			std::span<const char> data,
			Extent2D<uint32_t> extent, VkFormat format, VkImageUsageFlags usageFlags, VkImageLayout layout)
		{
			const auto stagingBuffer = CreateStagingBuffer(std::format("2D Texture {} staging buffer", name), vulkanContext, data.size());
			void* mappedData = vulkanContext.Map(*stagingBuffer);
			memcpy(mappedData, data.data(), data.size());
			vulkanContext.Unmap(*stagingBuffer);

			// @todo: Batched upload data
			// @todo: Use of Transfer Queue ; need more practicing and knowledge about pipeline barriers!
			auto& transferCmdPool = cmdPoolManager.RequestCommandPool(EQueueType::Graphics);
			const auto transferCmdBuffer = transferCmdPool.RequestCommandBuffer(std::format("Texture {} transfer cmd buffer", name));
			auto newTexture = CreateTexture2D(name, vulkanContext, extent, 1, format, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY);

			transferCmdBuffer->Begin();
			{
				transferCmdBuffer->ChangeImageAccessPattern(vk::EAccessPattern::None, vk::EAccessPattern::TransferWrite, newTexture->GetNativeHandle(), VK_IMAGE_ASPECT_COLOR_BIT);
				transferCmdBuffer->CopyBufferToImageSimple(*stagingBuffer, *newTexture);
				transferCmdBuffer->ChangeImageAccessPattern(vk::EAccessPattern::TransferWrite, vk::EAccessPattern::FragmentShaderReadSampledImage, newTexture->GetNativeHandle(), VK_IMAGE_ASPECT_COLOR_BIT);
			}
			transferCmdBuffer->End();

			const auto& uploadFence = frameTracker.GetCurrentInFlightUploadFence();
			vulkanContext.SubmitTo(*transferCmdBuffer, uploadFence);

			uploadFence.Wait();
			uploadFence.Reset();
			return newTexture;
		}

		std::unique_ptr<Texture> CreateTexture2DFromFile(std::string_view filePath,
			const VulkanContext& vulkanContext, const FrameTracker& frameTracker, CommandPoolManager& cmdPoolManager,
			VkFormat format, VkImageUsageFlags usageFlags, VkImageLayout layout)
		{
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
			auto result = CreateTexture2DFromMemory(
				filePath,
				vulkanContext, frameTracker, cmdPoolManager,
				std::span{ pixelsData, sizeOfData },
				Extent2D<uint32_t>{static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight)},
				format, usageFlags, layout);

			stbi_image_free(pixels);
			return result;
		}

		std::unique_ptr<Texture> CreateDepthStencil(std::string_view name,
			const VulkanContext& vulkanContext, const FrameTracker& frameTracker, CommandPoolManager& cmdPoolManager,
			Extent2D<uint32_t> extent)
		{
			auto res = CreateTexture2D(name, vulkanContext, extent, 1, VK_FORMAT_D24_UNORM_S8_UINT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VMA_MEMORY_USAGE_GPU_ONLY, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
			auto& transferCmdPool = cmdPoolManager.RequestCommandPool(EQueueType::Graphics);
			const auto transferCmdBuffer = transferCmdPool.RequestCommandBuffer(std::format("Depth-stencil buffer {} memory barrier cmd buffer", res->GetName()));

			transferCmdBuffer->Begin();
			{
				transferCmdBuffer->ChangeImageAccessPattern(vk::EAccessPattern::None, vk::EAccessPattern::DepthStencilAttachmentWrite, res->GetNativeHandle(), VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);
			}
			transferCmdBuffer->End();

			const auto& uploadFence = frameTracker.GetCurrentInFlightUploadFence();
			vulkanContext.SubmitTo(*transferCmdBuffer, uploadFence);
			uploadFence.Wait();
			uploadFence.Reset();
			return res;
		}
	}
}
