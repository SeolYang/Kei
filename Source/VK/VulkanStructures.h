#pragma once
#include <PCH.h>

namespace sy::vk
{
class VulkanRHI;

using Descriptor = OffsetSlotPtr;
class CommandBuffer;
using ManagedCommandBuffer = std::unique_ptr<CommandBuffer, std::function<void(CommandBuffer*)>>;
using VulkanObjectDeleter  = std::function<void(const VulkanRHI&)>;

struct TextureSubResource
{
    uint32_t MipLevel = 0;
    uint32_t ArrayLayer = 0;
};

struct BufferSubResource
{
    VkDeviceSize Offset = 0;
    VkDeviceSize Size = 0;
};
} // namespace sy::vk
