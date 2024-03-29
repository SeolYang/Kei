#pragma once
#include <PCH.h>

namespace sy::vk
{
class VulkanContext;
class PushConstantBuilder;
class PipelineLayoutCache : public Subsystem
{
public:
    struct PipelineLayoutInfo
    {
    public:
        bool   operator==(const PipelineLayoutInfo& rhs) const;
        size_t hash() const noexcept;

    public:
        VkPipelineLayoutCreateFlags        Flags;
        std::vector<VkDescriptorSetLayout> DescriptorSetLayouts;
        std::vector<VkPushConstantRange>   PushConstantRanges;
    };

    struct PipelineLayoutHash
    {
    public:
        size_t operator()(const PipelineLayoutInfo& info) const noexcept
        {
            return info.hash();
        }
    };

public:
    explicit PipelineLayoutCache(VulkanContext& vulkanContext);
    ~PipelineLayoutCache() override;

    void Startup() override;
    void Shutdown() override;

    VkPipelineLayout Request(std::span<VkDescriptorSetLayout> descriptorSetLayouts,
                             const PushConstantBuilder&       pushConstantBuilder);

private:
    VulkanContext&                                                                      vulkanContext;
    robin_hood::unordered_map<PipelineLayoutInfo, VkPipelineLayout, PipelineLayoutHash> cache;
};
} // namespace sy::vk
