#include <PCH.h>
#include <VK/Sampler.h>
#include <VK/VulkanRHI.h>

#include "SamplerBuilder.h"
#include "VulkanContext.h"

namespace sy::vk
{
	Sampler::Sampler(const SamplerBuilder& builder)
		: VulkanWrapper<VkSampler>(builder.name, builder.vulkanContext.GetRHI(), VK_OBJECT_TYPE_SAMPLER)
		, minFilter(builder.minFilter)
		, magFilter(builder.magFilter)
		, mipmapMode(builder.mipmapMode)
		, addressModeU(builder.addressModeU)
		, addressModeV(builder.addressModeV)
		, addressModeW(builder.addressModeW)
	{
		const VkSamplerCreateInfo samplerCreateInfo{
			.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
			.pNext = nullptr,
			.magFilter = magFilter,
			.minFilter = minFilter,
			.mipmapMode = mipmapMode,
			.addressModeU = addressModeU,
			.addressModeV = addressModeV,
			.addressModeW = addressModeW
		};

		NativeHandle handle = VK_NULL_HANDLE;
		VK_ASSERT(vkCreateSampler(GetRHI().GetDevice(), &samplerCreateInfo, nullptr, &handle), "Failed to create sampler {}.", builder.name);

		UpdateHandle(
			handle, SY_VK_WRAPPER_DELETER(rhi) {
				vkDestroySampler(rhi.GetDevice(), handle, nullptr);
			});
	}
} // namespace sy::vk
