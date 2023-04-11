#pragma once
#include <PCH.h>
#include <VK/Synchronization.h>

namespace sy::vk
{
class VulkanContext;
class VulkanRHI;
class Buffer;
class BufferStateTransition
{
public:
    explicit BufferStateTransition(const VulkanContext& vulkanContext);
    ~BufferStateTransition() = default;

	void SetHandle(const Handle<Buffer> handle)
	{
        this->handle = handle;
        this->nativeHandle = std::nullopt;
	}

	void SetNativeHandle(const VkBuffer buffer)
	{
        this->handle = {};
        this->nativeHandle = buffer;
	}

	void SetBuffer(const Buffer& buffer);

	bool IsUsedNativeHandle() const { return nativeHandle != std::nullopt; }

	void SetSourceState(const EBufferState state) { this->srcState = state; }
    void SetDestinationState(const EBufferState state) { this->dstState = state; }
    void SetSourceQueueType(const EQueueType queueType) { this->srcQueueType = queueType; }
    void SetDestinationQueueType(const EQueueType queueType) { this->dstQueueType = queueType; }
    void SetSubresourceRange(const Range<uint32_t> range) { this->subresourceRange = range; }

	VkBufferMemoryBarrier2 Build() const;

private:
    const VulkanContext& vulkanContext;
    Handle<Buffer> handle = {};
    std::optional<VkBuffer> nativeHandle = std::nullopt;
    std::optional<EBufferState> srcState = std::nullopt;
    std::optional<EBufferState> dstState = std::nullopt;
    std::optional<EQueueType> srcQueueType = std::nullopt;
    std::optional<EQueueType> dstQueueType = std::nullopt;
    std::optional<Range<uint32_t>> subresourceRange = std::nullopt;

};
} // namespace sy::vk