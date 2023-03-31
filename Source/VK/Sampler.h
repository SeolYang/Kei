#pragma once
#include <PCH.h>
#include <VK/VulkanWrapper.h>

namespace sy::vk
{
class SamplerBuilder;
class Sampler : public VulkanWrapper<VkSampler>
{
public:
    explicit Sampler(const SamplerBuilder& builder);
    ~Sampler() override = default;

private:
    const VkFilter             minFilter;
    const VkFilter             magFilter;
    const VkSamplerMipmapMode  mipmapMode;
    const VkSamplerAddressMode addressModeU;
    const VkSamplerAddressMode addressModeV;
    const VkSamplerAddressMode addressModeW;
};
} // namespace sy::vk
