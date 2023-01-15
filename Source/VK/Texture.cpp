#include <PCH.h>
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
		Texture::Texture(const std::string_view name, const VulkanContext& vulkanContext, const VkImageType type, const VkFormat format, const Extent3D<uint32_t> extent, const uint32_t mipLevels, uint32_t arrayLayers, const VkSampleCountFlagBits samples, const VkImageTiling tiling, VkImageUsageFlags usage, const VmaMemoryUsage memoryUsage, const VkMemoryPropertyFlags memoryProperties, const ETextureState initialState) :
			VulkanWrapper(name, vulkanContext, VK_OBJECT_TYPE_IMAGE, VK_DESTROY_LAMBDA_SIGNATURE(VkImage)
			{
		}),
			extent(extent),
			type(type),
			format(format),
			mipLevels(mipLevels),
			arrayLayers(arrayLayers),
			imageUsage(usage),
			memoryUsage(memoryUsage),
			memoryProperties(memoryProperties),
			initialState(initialState)
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
				.arrayLayers = arrayLayers,
				.samples = samples,
				.tiling = tiling,
				.usage = usage,
				.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
				.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
			};

			const VmaAllocationCreateInfo allocationCreateInfo
			{
				.usage = memoryUsage,
				.requiredFlags = memoryProperties
			};

			Native_t handle = VK_NULL_HANDLE;
			VK_ASSERT(vmaCreateImage(allocator, &imageCreateInfo, &allocationCreateInfo, &handle, &allocation, nullptr), "Failed to create image {}.", name);
			UpdateHandle(handle);
		}

		Texture::~Texture()
		{
			if (allocation != VK_NULL_HANDLE)
			{
				const auto& vulkanContext = GetContext();
				const auto handle = GetNativeHandle();
				vmaDestroyImage(vulkanContext.GetAllocator(), handle, allocation);
			}
		}

		std::unique_ptr<Texture> CreateTexture2D(std::string_view name, const VulkanContext& vulkanContext,
			Extent2D<uint32_t> extent, uint32_t mipLevels, VkFormat format, VkImageUsageFlags usages,
			VmaMemoryUsage memoryUsage, VkMemoryPropertyFlags memoryProperties, const ETextureState initialState)
		{
			return std::make_unique<Texture>(name, vulkanContext, VK_IMAGE_TYPE_2D, format, Extent3D<uint32_t>{extent.width, extent.height, 1}, mipLevels, 1, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_TILING_OPTIMAL, usages, memoryUsage, memoryProperties, initialState);
		}

		std::unique_ptr<Texture> CreateTexture2DFromMemory(std::string_view name, const VulkanContext& vulkanContext,
			const FrameTracker& frameTracker, CommandPoolManager& cmdPoolManager,
			std::span<const char> data,
			Extent2D<uint32_t> extent, VkFormat format)
		{
			const auto stagingBuffer = CreateStagingBuffer(std::format("2D Texture {} staging buffer", name), vulkanContext, data.size());
			void* mappedData = vulkanContext.Map(*stagingBuffer);
			memcpy(mappedData, data.data(), data.size());
			vulkanContext.Unmap(*stagingBuffer);

			// @todo: Batched upload data
			// @todo: Use of Transfer Queue ; need more practicing and knowledge about pipeline barriers!
			auto& transferCmdPool = cmdPoolManager.RequestCommandPool(EQueueType::Graphics);
			const auto transferCmdBuffer = transferCmdPool.RequestCommandBuffer(std::format("Texture {} transfer cmd buffer", name));
			auto newTexture = CreateTexture2D(name, vulkanContext, extent, 1, format, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY, 0, vk::ETextureState::AnyShaderReadSampledImage);

			transferCmdBuffer->Begin();
			{
				transferCmdBuffer->ChangeState(vk::ETextureState::None, vk::ETextureState::TransferWrite, *newTexture);
				transferCmdBuffer->CopyBufferToImageSimple(*stagingBuffer, *newTexture);
				transferCmdBuffer->ChangeState(vk::ETextureState::TransferWrite, vk::ETextureState::AnyShaderReadSampledImage, *newTexture);
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
			VkFormat format)
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
				format);

			stbi_image_free(pixels);
			return result;
		}

		std::unique_ptr<Texture> CreateDepthStencil(std::string_view name,
			const VulkanContext& vulkanContext, const FrameTracker& frameTracker, CommandPoolManager& cmdPoolManager,
			Extent2D<uint32_t> extent)
		{
			auto res = CreateTexture2D(name, vulkanContext, extent, 1, VK_FORMAT_D24_UNORM_S8_UINT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VMA_MEMORY_USAGE_GPU_ONLY, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, ETextureState::DepthStencilAttachmentWrite);
			auto& transferCmdPool = cmdPoolManager.RequestCommandPool(EQueueType::Graphics);
			const auto transferCmdBuffer = transferCmdPool.RequestCommandBuffer(std::format("Depth-stencil buffer {} memory barrier cmd buffer", res->GetName()));

			transferCmdBuffer->Begin();
			{
				transferCmdBuffer->ChangeState(vk::ETextureState::None, vk::ETextureState::DepthStencilAttachmentWrite, *res);
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
