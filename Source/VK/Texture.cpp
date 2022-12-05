#include <Core.h>
#include <VK/VulkanContext.h>
#include <VK/Fence.h>
#include <VK/Texture.h>
#include <VK/Buffer.h>
#include <VK/CommandPool.h>
#include <VK/CommandBuffer.h>
#include <CommandPoolManager.h>
#include <FrameTracker.h>

namespace sy
{
	Texture::Texture(const std::string_view name, const VulkanContext& vulkanContext, const Extent3D<uint32_t> extent, const uint32_t mipLevels, const VkFormat format, const VkImageUsageFlags usageFlags, const VkImageLayout desiredImageLayout, const VmaMemoryUsage memoryUsage, const VkImageAspectFlags imageAspect, const VkMemoryPropertyFlags requiredMemoryPropertyFlags) :
		VulkanWrapper(name, vulkanContext, VK_OBJECT_TYPE_IMAGE, VK_DESTROY_LAMBDA_SIGNATURE(VkImage)
		{
		}),
		extent(extent),
		format(format),
		mipLevels(mipLevels),
		desiredImageLayout(desiredImageLayout),
		imageUsage(usageFlags),
		memoryUsage(memoryUsage),
		imageAspect(imageAspect),
		memoryProperty(requiredMemoryPropertyFlags)
	{
		const VkSamplerCreateInfo samplerCreateInfo
		{
			.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
			.pNext = nullptr,
			.magFilter = VK_FILTER_LINEAR,
			.minFilter = VK_FILTER_LINEAR,
			.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
			.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
			.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
			.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT
		};
		VK_ASSERT(vkCreateSampler(vulkanContext.GetDevice(), &samplerCreateInfo, nullptr, &sampler), "Failed to create sampler.");
	}

	Texture::~Texture()
	{
		if (view != VK_NULL_HANDLE)
		{
			vkDestroyImageView(vulkanContext.GetDevice(), view, nullptr);
		}

		if (sampler != VK_NULL_HANDLE)
		{
			vkDestroySampler(vulkanContext.GetDevice(), sampler, nullptr);
		}

		if (handle != VK_NULL_HANDLE)
		{
			vmaDestroyImage(vulkanContext.GetAllocator(), handle, allocation);
		}
	}

	Texture2D::Texture2D(std::string_view name, const VulkanContext& vulkanContext, const Extent2D<uint32_t> extent, const uint32_t mipLevels, const VkFormat format,
		const VkImageUsageFlags usageFlags, const VkImageLayout desiredImageLayout, const VmaMemoryUsage memoryUsage, const VkImageAspectFlags imageAspect, const VkMemoryPropertyFlags requiredMemoryPropertyFlags) :
		Texture(name, vulkanContext, { extent.width, extent.height, 1 }, mipLevels, format, usageFlags, desiredImageLayout, memoryUsage, imageAspect, requiredMemoryPropertyFlags)
	{
		const auto allocator = vulkanContext.GetAllocator();
		const VkImageCreateInfo imageCreateInfo
		{
			.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.imageType = VK_IMAGE_TYPE_2D,
			.format = format,
			.extent = VkExtent3D{extent.width, extent.height, 1},
			.mipLevels = mipLevels,
			.arrayLayers = 1,
			.samples = VK_SAMPLE_COUNT_1_BIT,
			.tiling = VK_IMAGE_TILING_OPTIMAL,
			.usage = usageFlags,
			.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
			.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
		};

		const VmaAllocationCreateInfo allocationCreateInfo
		{
			.usage = memoryUsage,
			.requiredFlags = requiredMemoryPropertyFlags
		};
		VK_ASSERT(vmaCreateImage(allocator, &imageCreateInfo, &allocationCreateInfo, &handle, &allocation, nullptr), "Failed to create image {}.", name);

		const VkImageViewCreateInfo viewCreateInfo
		{
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.image = handle,
			.viewType = VK_IMAGE_VIEW_TYPE_2D,
			.format = format,
			.subresourceRange = VkImageSubresourceRange
			{
				.aspectMask = imageAspect,
				.baseMipLevel = 0,
				.levelCount = mipLevels,
				.baseArrayLayer = 0,
				.layerCount = 1
			}
		};
		VK_ASSERT(vkCreateImageView(vulkanContext.GetDevice(), &viewCreateInfo, nullptr, &view), "Failed to create image view {}.", name);
	}

	std::unique_ptr<Texture2D> Texture2D::LoadFromFile(CommandPoolManager& cmdPoolManager, const FrameTracker& frameTracker, const std::string_view filePath, const VulkanContext& vulkanContext,
		const VkFormat format, const VkImageUsageFlags usageFlags, const VkImageLayout layout)
	{
		int texWidth, texHeight;
		int texChannels;
		stbi_uc* pixels = stbi_load(filePath.data(), &texWidth, &texHeight, &texChannels, static_cast<int>(ToNumberOfComponents(format)));
		if (pixels == nullptr)
		{
			spdlog::warn("Failed to load image from file {}.", filePath);
			return nullptr;
		}

		const void* pixelsData = pixels;
		const VkDeviceSize sizeOfData = (static_cast<uint64_t>(texWidth) * static_cast<uint64_t>(texHeight)) * ToByteSize(format);

		const auto stagingBuffer = Buffer::CreateStagingBuffer(std::format("2D Texture {} staging buffer", filePath), vulkanContext, sizeOfData);
		void* mappedData = vulkanContext.Map(*stagingBuffer);
		memcpy(mappedData, pixelsData, sizeOfData);
		vulkanContext.Unmap(*stagingBuffer);
		stbi_image_free(pixels);

		// @todo: Batched upload data
		// @todo: Use of Transfer Queue ; need more practicing and knowledge about pipeline barriers!
		auto& transferCmdPool = cmdPoolManager.RequestCommandPool(EQueueType::Graphics);
		const auto transferCmdBuffer = transferCmdPool.RequestCommandBuffer(std::format("Texture {} transfer cmd buffer", filePath));
		std::unique_ptr<Texture2D> newTexture = std::make_unique<Texture2D>(filePath, vulkanContext, Extent2D<uint32_t>{ static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight) }, 1, format, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VMA_MEMORY_USAGE_GPU_ONLY);

		transferCmdBuffer->Begin();
		{
			std::array transferTexBarrier = { vkinit::ImageMemoryBarrier(0, VK_ACCESS_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, newTexture->GetNativeHandle(), vkinit::ImageSubresourceRange()) };
			transferCmdBuffer->PipelineBarrier(VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, {}, {}, transferTexBarrier);
			transferCmdBuffer->CopyBufferToImageSimple(*stagingBuffer, *newTexture);
			std::array readableTexBarrier = { vkinit::ImageMemoryBarrier(VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, newTexture->GetNativeHandle(), vkinit::ImageSubresourceRange()) };
			transferCmdBuffer->PipelineBarrier(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, {}, {}, readableTexBarrier);
		}
		transferCmdBuffer->End();

		const auto& uploadFence = frameTracker.GetCurrentInFlightUploadFence();
		vulkanContext.SubmitTo(*transferCmdBuffer, uploadFence);
		uploadFence.Wait();
		uploadFence.Reset();
		return newTexture;
	}

	std::unique_ptr<Texture2D> Texture2D::CreateDepthStencil(CommandPoolManager& cmdPoolManager, const FrameTracker& frameTracker, std::string_view name, const VulkanContext& vulkanContext, const Extent2D<uint32_t> extent)
	{
		auto res = std::make_unique<Texture2D>("Depth-Stencil", vulkanContext, extent, 1, VK_FORMAT_D24_UNORM_S8_UINT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VMA_MEMORY_USAGE_GPU_ONLY, VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		auto& transferCmdPool = cmdPoolManager.RequestCommandPool(EQueueType::Graphics);
		const auto transferCmdBuffer = transferCmdPool.RequestCommandBuffer(std::format("Depth-stencil buffer {} memory barrier cmd buffer", res->GetName()));

		transferCmdBuffer->Begin();
		{
			std::array transferTexBarrier = { vkinit::ImageMemoryBarrier(0,  VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, VK_IMAGE_LAYOUT_UNDEFINED, res->GetDesiredLayout(), res->GetNativeHandle(), vkinit::ImageSubresourceRange(VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT)) };
			transferCmdBuffer->PipelineBarrier(VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT, {}, {}, transferTexBarrier);
		}
		transferCmdBuffer->End();

		const auto& uploadFence = frameTracker.GetCurrentInFlightUploadFence();
		vulkanContext.SubmitTo(*transferCmdBuffer, uploadFence);
		uploadFence.Wait();
		uploadFence.Reset();
		return res;
	}
}
