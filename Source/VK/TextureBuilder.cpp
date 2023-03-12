#include <PCH.h>
#include <VK/TextureBuilder.h>
#include <VK/Texture.h>

namespace sy::vk
{
	std::unique_ptr<Texture> TextureBuilder::Build() const
	{
		return std::make_unique<Texture>(*this);
	}

	TextureBuilder TextureBuilder::Texture2DShaderResourceTemplate(const VulkanContext& vulkanContext)
	{
		return TextureBuilder{ vulkanContext }
			.SetType(VK_IMAGE_TYPE_2D)
			.SetUsage(VK_IMAGE_USAGE_SAMPLED_BIT)
			.SetMemoryUsage(VMA_MEMORY_USAGE_GPU_ONLY)
			.SetMemoryProperty(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
			.SetTargetInitialState(ETextureState::AnyShaderReadGeneral);
	}

	TextureBuilder TextureBuilder::Texture2DRenderTargetTemplate(const VulkanContext& vulkanContext)
	{
		return TextureBuilder{ vulkanContext }
			.SetType(VK_IMAGE_TYPE_2D)
			.SetUsage(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
			.SetMemoryUsage(VMA_MEMORY_USAGE_GPU_ONLY)
			.SetMemoryProperty(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
			.SetTargetInitialState(ETextureState::ColorAttachmentReadWrite);
	}

	TextureBuilder TextureBuilder::Texture2DDepthStencilTemplate(const VulkanContext& vulkanContext)
	{
		return TextureBuilder{ vulkanContext }
			.SetType(VK_IMAGE_TYPE_2D)
			.SetUsage(VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
			.SetMemoryUsage(VMA_MEMORY_USAGE_GPU_ONLY)
			.SetMemoryProperty(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
			.SetTargetInitialState(ETextureState::DepthStencilAttachmentWrite);
	}
}
