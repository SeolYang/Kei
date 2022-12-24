#include <Core/Core.h>
#include <VK/Sampler.h>

#include "VulkanContext.h"

namespace sy::vk
{
	Sampler::Sampler(std::string_view name, const VulkanContext& vulkanContext, const VkFilter filter, const VkSamplerMipmapMode mipmapMode, const VkSamplerAddressMode addressMode) :
		VulkanWrapper<VkSampler>(name, vulkanContext, VK_OBJECT_TYPE_SAMPLER, VK_DESTROY_LAMBDA_SIGNATURE(VkSampler)
		{
			vkDestroySampler(vulkanContext.GetDevice(), handle, nullptr);
		})
	{
		const VkSamplerCreateInfo samplerCreateInfo
		{
			.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
			.pNext = nullptr,
			.magFilter = filter,
			.minFilter = filter,
			.mipmapMode = mipmapMode,
			.addressModeU = addressMode,
			.addressModeV = addressMode,
			.addressModeW = addressMode
		};
		VK_ASSERT(vkCreateSampler(vulkanContext.GetDevice(), &samplerCreateInfo, nullptr, &handle), "Failed to create sampler {}.", name);
	}
}
