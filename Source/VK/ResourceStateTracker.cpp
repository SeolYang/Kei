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
			if (const auto resourceRef = resourceCache.Load(handle); resourceRef)
			{
				const auto& resource = resourceRef.value().get();
				TextureState& state = foundItr->second;
				if (!subResources.empty())
				{
					if (!state.bTrackingPerSubResource)
					{
						state.bTrackingPerSubResource = true;
					}

					for (const auto& subResource : subResources)
					{
						const size_t subResourceIndex = ResolveTextureSubResourceIndex(subResource.MipLevel, subResource.ArrayLayer, subResource.NumMips);
						if (state.SubResourceStates[subResourceIndex] != dstState)
						{
							state.SubResourceStates[subResourceIndex] = dstState;
							pendingTextureTransitions.emplace_back(resource, state.State, dstState, subResource.MipLevel, 1, subResource.ArrayLayer, 1);
						}
					}
				}
				else
				{
					const size_t arrayLayerCount = (resource.IsTextureArray() ? resource.GetExtent().depth : 1);
					if (state.bTrackingPerSubResource)
					{
						for (size_t arrayLayer = 0; arrayLayer < arrayLayerCount; ++arrayLayer)
						{
							for (size_t mipLevel = 0; mipLevel < resource.GetMipLevels(); ++mipLevel)
							{
								const size_t subResourceIndex = ResolveTextureSubResourceIndex(mipLevel, arrayLayer, resource.GetMipLevels());
								if (state.SubResourceStates[subResourceIndex] != dstState)
								{
									pendingTextureTransitions.emplace_back(resource, state.SubResourceStates[subResourceIndex], dstState, mipLevel, 1, arrayLayer, 1);
									state.SubResourceStates[subResourceIndex] = dstState;
								}
							}
						}
					}
					else
					{
						pendingTextureTransitions.emplace_back(resource, state.State, dstState, 0, resource.GetMipLevels(), 0, arrayLayerCount);
					}

					state.State = dstState;
					state.bTrackingPerSubResource = false;
				}
			}
		}
	}

	void ResourceStateTracker::PendingTransition(const ETextureState dstState, const Handle<Texture> handle)
	{
		if (const auto resourceRef = resourceCache.Load(handle); resourceRef)
		{
			const auto& resource = resourceRef.value().get();
			/** @TODO Handle texture arrays */
			PendingTransition(dstState, handle, {});
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
			PendingTransition(dstState, handle, 0, resource.GetSize());
		}
	}
}
