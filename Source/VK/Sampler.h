#pragma once
#include <Core/Core.h>

namespace sy::vk
{
	class VulkanContext;
	class Sampler : public VulkanWrapper<VkSampler>
	{
	public:
		Sampler(std::string_view name, const VulkanContext& vulkanContext, VkFilter filter = VK_FILTER_LINEAR, VkSamplerMipmapMode mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR, VkSamplerAddressMode addressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT);
		virtual ~Sampler() override = default;

	};
}