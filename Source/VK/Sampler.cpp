#include <PCH.h>
#include <VK/Sampler.h>
#include <VK/VulkanContext.h>

namespace sy::vk
{
	Sampler::Sampler(std::string_view name, const VulkanContext& vulkanContext, const SamplerInfo info) :
		VulkanWrapper<VkSampler>(name, vulkanContext, VK_OBJECT_TYPE_SAMPLER, VK_DESTROY_LAMBDA_SIGNATURE(VkSampler)
		{
			vkDestroySampler(vulkanContext.GetDevice(), handle, nullptr);
		})
	{
		const VkSamplerCreateInfo samplerCreateInfo
		{
			.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
			.pNext = nullptr,
			.magFilter = info.MagnificationFilter,
			.minFilter = info.MinificationFilter,
			.mipmapMode = info.MipmapMode,
			.addressModeU = info.AddressModeU,
			.addressModeV = info.AddressModeV,
			.addressModeW = info.AddressModeW
		};

		Native_t handle = VK_NULL_HANDLE;
		VK_ASSERT(vkCreateSampler(vulkanContext.GetDevice(), &samplerCreateInfo, nullptr, &handle), "Failed to create sampler {}.", name);
		UpdateHandle(handle);
	}
}
