#pragma once
#include <PCH.h>

namespace sy::vk
{
	constexpr std::string_view LinearSamplerRepeat = "LinearSamplerRepeat";
	struct SamplerInfo
	{
		VkFilter MinificationFilter = VK_FILTER_LINEAR;
		VkFilter MagnificationFilter = VK_FILTER_LINEAR;
		VkSamplerMipmapMode MipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		VkSamplerAddressMode AddressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		VkSamplerAddressMode AddressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		VkSamplerAddressMode AddressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	};

	class VulkanContext;
	class Sampler : public VulkanWrapper<VkSampler>
	{
	public:
		Sampler(std::string_view name, const VulkanContext& vulkanContext, SamplerInfo info);
		virtual ~Sampler() override = default;

	private:
		SamplerInfo info;

	};
}