#include <PCH.h>
#include <VK/ResourceStateTracker.h>
#include <VK/Texture.h>
#include <VK/Buffer.h>
#include <Core/ResourceCache.h>

namespace sy::vk
{
	ResourceStateTracker::ResourceStateTracker(ResourceCache& resourceCache) :
	resourceCache(resourceCache)
	{
	}

	void ResourceStateTracker::Register(const Handle<Texture> handle)
	{
		if (textureStates.find(handle.Value) == textureStates.end())
		{
			if (const auto resourceRef = resourceCache.Load(handle); resourceRef)
			{
				const auto& resource = resourceRef.value().get();

				TextureState newState;
				newState.Handle = handle;
				newState.State = resource.GetInitialState();
				newState.SubResourceStates.resize(resource.GetNumSubResources(), resource.GetInitialState());
				textureStates[handle.Value] = newState;
			}
		}
	}

	void ResourceStateTracker::UnRegister(const Handle<Texture> handle)
	{
		textureStates.erase(handle.Value);
	}

	void ResourceStateTracker::Register(const Handle<Buffer> handle)
	{
		if (bufferStates.find(handle.Value) == bufferStates.end())
		{
			if (const auto resourceRef = resourceCache.Load(handle); resourceRef)
			{
				const auto& resource = resourceRef.value().get();
				bufferStates[handle.Value] = { handle, resource.GetInitialState() };
			}
		}
	}

	void ResourceStateTracker::UnRegister(const Handle<Buffer> handle)
	{
		bufferStates.erase(handle.Value);
	}

	void ResourceStateTracker::PendingTransition(const ETextureState dstState, const Handle<Texture> handle, const std::span<const TextureSubResource> subResources)
	{
		auto foundItr = textureStates.find(handle.Value);
		if (foundItr != textureStates.end())
		{
			if (const auto resourceOpt = resourceCache.Load(handle); resourceOpt)
			{
				const auto& resource = Unwrap(resourceOpt);
				TextureState& state = foundItr->second;
				if (!subResources.empty())
				{
					if (!state.bTrackingPerSubResource)
					{
						state.bTrackingPerSubResource = true;
					}

					for (const auto& subResource : subResources)
					{
						const size_t subResourceIndex = ResolveTextureSubResourceRangeIndex(subResource.MipLevel, subResource.ArrayLayer, resource.GetMipLevels());
						if (state.SubResourceStates[subResourceIndex] != dstState)
						{
							state.SubResourceStates[subResourceIndex] = dstState;
							pendingTextureTransitions.emplace_back(resource, state.State, dstState, TextureSubResourceRange{ .MipLevel = subResource.MipLevel, .ArrayLayer = subResource.ArrayLayer });
						}
					}
				}
				else
				{
					if (state.bTrackingPerSubResource)
					{
						for (uint32_t arrayLayer = 0; arrayLayer < resource.GetArrayLayers(); ++arrayLayer)
						{
							for (uint32_t mipLevel = 0; mipLevel < resource.GetMipLevels(); ++mipLevel)
							{
								const size_t subResourceIndex = ResolveTextureSubResourceRangeIndex(mipLevel, arrayLayer, resource.GetMipLevels());
								if (state.SubResourceStates[subResourceIndex] != dstState)
								{
									pendingTextureTransitions.emplace_back(resource, state.SubResourceStates[subResourceIndex], dstState, TextureSubResourceRange{ .MipLevel = mipLevel, .ArrayLayer = arrayLayer });
									state.SubResourceStates[subResourceIndex] = dstState;
								}
							}
						}
					}
					else
					{
						pendingTextureTransitions.emplace_back(resource, state.State, dstState, resource.GetFullSubResourceRange());
					}

					state.State = dstState;
					state.bTrackingPerSubResource = false;
				}
			}
		}
	}

	void ResourceStateTracker::PendingTransition(const EBufferState dstState, const Handle<Buffer> handle, const size_t offset, const size_t size) 
	{
		auto foundItr = bufferStates.find(handle.Value);
		if (foundItr != bufferStates.end())
		{
			if (const auto resourceRef = resourceCache.Load(handle); resourceRef)
			{
				const auto& resource = resourceRef.value().get();
				BufferState& state = foundItr->second;
				if (state.State != dstState)
				{
					pendingBufferTransitions.emplace_back(resource, state.State, dstState);
					state.State = dstState;
				}
			}
		}
	}

	void ResourceStateTracker::PendingTransition(const EBufferState dstState, const Handle<Buffer> handle)
	{
		if (const auto resourceRef = resourceCache.Load(handle); resourceRef)
		{
			const auto& resource = resourceRef.value().get();
			PendingTransition(dstState, handle, 0, resource.GetAlignedSize());
		}
	}
}
