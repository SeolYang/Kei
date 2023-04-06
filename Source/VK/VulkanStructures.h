#pragma once
#include <PCH.h>

namespace sy::vk
{
class VulkanRHI;

using Descriptor = OffsetSlotPtr;
class CommandBuffer;
using ManagedCommandBuffer = std::unique_ptr<CommandBuffer, std::function<void(CommandBuffer*)>>;
using VulkanObjectDeleter  = std::function<void(const VulkanRHI&)>;

struct TextureSubResourceRange
{
    uint32_t MipLevel        = 0;
    uint32_t MipLevelCount   = 1;
    uint32_t ArrayLayer      = 0;
    uint32_t ArrayLayerCount = 1;
};

struct TextureSubResource
{
    uint32_t MipLevel   = 0;
    uint32_t ArrayLayer = 0;
};
} // namespace sy::vk
