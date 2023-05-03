#pragma once
#include <PCH.h>
#include <VK/Synchronization.h>

namespace sy::vk
{
class VulkanContext;
class VulkanRHI;
template <typename Resource>
class ResourceStateTransition
{
public:
    using NativeHandle = Resource::NativeHandle;
    using SubresourceRange = Resource::SubresourceRange;
    using Barrier = Resource::Barrier;
    using State = Resource::State;

public:
    explicit ResourceStateTransition(const VulkanContext& vulkanContext) :
        vulkanContext(vulkanContext)
    {
    }

    ~ResourceStateTransition() = default;

    void SetHandle(const Handle<Resource> newHandle)
    {
        handle = newHandle;
        nativeHandle = std::nullopt;
    }

    void SetNativeHandle(const NativeHandle newNativeHandle)
    {
        handle = {};
        nativeHandle = newNativeHandle;
    }

    void SetSubresourceRange(const SubresourceRange newSubresourceRange)
    {
        subresourceRange = newSubresourceRange;
    }

    void SetResource(const Resource& resource)
    {
        SetNativeHandle(resource.GetNative());
        SetSubresourceRange(resource.GetFullSubresourceRange());
    }

    void SetSourceState(const State state) { srcAccessPattern = QueryAccessPattern(state); }
    void SetDestinationState(const State state) { dstAccessPattern = QueryAccessPattern(state); }

    void OverlapSourceState(const State state)
    {
        if (srcAccessPattern)
        {
            srcAccessPattern->Overlap(QueryAccessPattern(state));
        }
        else
        {
            SetSourceState(state);
        }
    }

    void OverlapDestinationState(const State state)
    {
        if (dstAccessPattern)
        {
            dstAccessPattern->Overlap(QueryAccessPattern(state));
        }
        else
        {
            SetDestinationState(state);
        }
    }

    void SetSourceQueueType(const EQueueType srcQueueType) { this->srcQueueType = srcQueueType; }
    void SetDestinationQueueType(const EQueueType dstQueueType) { this->dstQueueType = dstQueueType; }

    bool IsUsedNativeHandle() const { return nativeHandle != std::nullopt; }

    Barrier Build() const;

private:
    void CheckValidation() const
    {
        const bool bValidStates = srcAccessPattern && dstAccessPattern;
        SY_ASSERT(bValidStates, "Both states are does not setup. It result in redundant transition.");
        const bool bStatesAreNotEqual =
            (srcAccessPattern->Access != dstAccessPattern->Access) ||
            (srcAccessPattern->ImageLayout != dstAccessPattern->ImageLayout) ||
            (srcAccessPattern->PipelineStage != dstAccessPattern->PipelineStage);
        SY_ASSERT(bStatesAreNotEqual, "State are equal. It may result in redudant transition.");

        bool bIsValidNativeHandle = false;
        if (IsUsedNativeHandle())
        {
            bIsValidNativeHandle = nativeHandle != VK_NULL_HANDLE;
            SY_ASSERT(bIsValidNativeHandle, "Invalid Vulkan Native Handle.");

            const bool bHasSubresourceRange = subresourceRange != std::nullopt;
            SY_ASSERT(bHasSubresourceRange, "Native Handle State Transtion must specify subresource range manually.");
        }
        else
        {
            const bool bIsValidHandle = handle.IsValid();
            SY_ASSERT(bIsValidHandle, "Invalid Texture Handle.");
            bIsValidNativeHandle = handle->GetNative() != VK_NULL_HANDLE;
        }
        SY_ASSERT(bIsValidNativeHandle, "Invalid Vulkan Native Handle.");
    }

private:
    const VulkanContext& vulkanContext;
    Handle<Resource> handle = {};
    std::optional<NativeHandle> nativeHandle = std::nullopt;
    std::optional<AccessPattern> srcAccessPattern = std::nullopt;
    std::optional<AccessPattern> dstAccessPattern = std::nullopt;
    std::optional<EQueueType> srcQueueType = std::nullopt;
    std::optional<EQueueType> dstQueueType = std::nullopt;
    std::optional<SubresourceRange> subresourceRange = std::nullopt;
};
} // namespace sy::vk