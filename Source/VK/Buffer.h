#pragma once
#include <PCH.h>
#include <VK/VulkanWrapper.h>

namespace sy::vk
{
class BufferBuilder;

class Buffer : public VulkanWrapper<VkBuffer>
{
public:
    explicit Buffer(const BufferBuilder& builder);
    ~Buffer() override = default;

    [[nodiscard]] auto GetAlignedSize() const { return this->alignedSize; }
    [[nodiscard]] auto GetUsage() const { return this->usage; }
    [[nodiscard]] VmaMemoryUsage GetMemoryUsage() const { return this->memoryUsage; }
    [[nodiscard]] auto GetInitialState() const { return this->initialState; }
    [[nodiscard]] auto GetDescriptorInfo(const size_t offset = 0) const { return VkDescriptorBufferInfo{GetNative(), offset, GetAlignedSize()}; }
    [[nodiscard]] const VmaAllocation& GetAllocation() const { return allocation; }
    [[nodiscard]] Range<uint32_t> GetFullSubresourceRange() const { return Range<uint32_t>{0, static_cast<uint32_t>(alignedSize)}; }

private:
    VmaAllocation allocation = VK_NULL_HANDLE;
    /** It can be extend its size later maybe? */
    const size_t alignedSize;
    const VkBufferUsageFlags usage;
    const VmaMemoryUsage memoryUsage;
    const EBufferState initialState;
};
} // namespace sy::vk
