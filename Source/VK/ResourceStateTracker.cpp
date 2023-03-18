#include <PCH.h>
#include <VK/ResourceStateTracker.h>
#include <VK/Texture.h>
#include <VK/Buffer.h>

namespace sy::vk
{
ResourceStateTracker::ResourceStateTracker(HandleManager& handleManager) :
    handleManager(handleManager)
{
}

void ResourceStateTracker::Register(const Handle<Texture> handle)
{
    const auto placement = handle.GetPlacement();
    if (textureStates.find(placement) == textureStates.end())
    {
        if (handle)
        {
            textureStates[placement] = {
                .Handle            = handle,
                .State             = handle->GetInitialState(),
                .SubResourceStates = std::vector{handle->GetNumSubResources(), handle->GetInitialState()}};
        }
    }
}

void ResourceStateTracker::UnRegister(const Handle<Texture> handle)
{
    textureStates.erase(handle.GetPlacement());
}

void ResourceStateTracker::Register(const Handle<Buffer> handle)
{
    if (bufferStates.find(handle.GetPlacement()) == bufferStates.end())
    {
        if (handle)
        {
            bufferStates[handle.GetPlacement()] = {handle, handle->GetInitialState()};
        }
    }
}

void ResourceStateTracker::UnRegister(const Handle<Buffer> handle)
{
    bufferStates.erase(handle.GetPlacement());
}

void ResourceStateTracker::PendingTransition(const ETextureState dstState, const Handle<Texture> handle, const std::span<const TextureSubResource> subResources)
{
    auto foundItr = textureStates.find(handle.GetPlacement());
    if (foundItr != textureStates.end())
    {
        if (handle)
        {
            TextureState& state = foundItr->second;
            if (!subResources.empty())
            {
                if (!state.bTrackingPerSubResource)
                {
                    state.bTrackingPerSubResource = true;
                }

                for (const auto& subResource : subResources)
                {
                    const size_t subResourceIndex = ResolveTextureSubResourceRangeIndex(subResource.MipLevel,
                                                                                        subResource.ArrayLayer, handle->GetMipLevels());
                    if (state.SubResourceStates[subResourceIndex] != dstState)
                    {
                        state.SubResourceStates[subResourceIndex] = dstState;
                        pendingTextureTransitions.emplace_back(*handle, state.State, dstState,
                                                               TextureSubResourceRange{
                                                                   .MipLevel   = subResource.MipLevel,
                                                                   .ArrayLayer = subResource.ArrayLayer});
                    }
                }
            }
            else
            {
                if (state.bTrackingPerSubResource)
                {
                    for (uint32_t arrayLayer = 0; arrayLayer < handle->GetArrayLayers(); ++arrayLayer)
                    {
                        for (uint32_t mipLevel = 0; mipLevel < handle->GetMipLevels(); ++mipLevel)
                        {
                            const size_t subResourceIndex =
                                ResolveTextureSubResourceRangeIndex(mipLevel, arrayLayer,
                                                                    handle->GetMipLevels());
                            if (state.SubResourceStates[subResourceIndex] != dstState)
                            {
                                pendingTextureTransitions.emplace_back(*handle,
                                                                       state.SubResourceStates[subResourceIndex],
                                                                       dstState, TextureSubResourceRange{.MipLevel = mipLevel, .ArrayLayer = arrayLayer});
                                state.SubResourceStates[subResourceIndex] = dstState;
                            }
                        }
                    }
                }
                else
                {
                    pendingTextureTransitions.emplace_back(*handle, state.State, dstState,
                                                           handle->GetFullSubResourceRange());
                }

                state.State                   = dstState;
                state.bTrackingPerSubResource = false;
            }
        }
    }
}

void ResourceStateTracker::PendingTransition(const EBufferState dstState, const Handle<Buffer> handle, const size_t offset, const size_t size)
{
    auto foundItr = bufferStates.find(handle.GetPlacement());
    if (foundItr != bufferStates.end())
    {
        if (handle)
        {
            BufferState& state = foundItr->second;
            if (state.State != dstState)
            {
                pendingBufferTransitions.emplace_back(*handle, state.State, dstState);
                state.State = dstState;
            }
        }
    }
}

void ResourceStateTracker::PendingTransition(const EBufferState dstState, const Handle<Buffer> handle)
{
    if (handle)
    {
        PendingTransition(dstState, handle, 0, handle->GetAlignedSize());
    }
}
} // namespace sy::vk
