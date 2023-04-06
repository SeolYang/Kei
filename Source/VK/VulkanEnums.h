#pragma once

namespace sy::vk
{
enum class EQueueType
{
    Graphics,
    Compute,
    Transfer,
    Present
};

enum class EPipelineType
{
    Graphics,
    Compute,
    RayTracing
};

enum class EDescriptorType : uint8_t
{
    Sampler = 0,
    SampledImage,
    CombinedImageSampler,
    StorageImage,
    UniformBuffer,
    StorageBuffer,
    InputAttachment,
    UniformBufferDynamic,
    StorageBufferDynamic,
    EnumMax
};
}