#pragma once
#include <PCH.h>

namespace sy
{
	class CacheRegistry;
}

namespace sy::vk
{
	inline auto ResolveTextureSubResourceIndex(const size_t mipLevel, const size_t arrayLayer, const size_t numMipLevels)
	{
		return (arrayLayer * numMipLevels) + mipLevel;
	}

	class Texture;
	class Buffer;
	class ResourceStateTracker : public NonCopyable
	{
	public:
		struct TextureState
		{
			Handle<Texture> Handle = {};
			ETextureState State = ETextureState::None;
			std::vector<ETextureState> SubResourceStates;
			bool bTrackingPerSubResource = false;
		};

		struct TextureSubResource
		{
			uint32_t MipLevel = 0;
			uint32_t NumMips = 1;
			uint32_t ArrayLayer = 0;
		};

		struct BufferState
		{
			Handle<Buffer> Handle = {};
			EBufferState State = EBufferState::None;
		};

	public:
		ResourceStateTracker(CacheRegistry& cacheRegistry);
		~ResourceStateTracker() override = default;

		void Register(Handle<Texture> handle);
		void UnRegister(Handle<Texture> handle);
		void Register(Handle<Buffer> handle);
		void UnRegister(Handle<Buffer> handle);

		void PendingTransition(ETextureState dstState, Handle<Texture> handle, std::span<const TextureSubResource> subResources);
		void PendingTransition(ETextureState dstState, Handle<Texture> handle);
		void PendingTransition(EBufferState dstState, Handle<Buffer> handle, size_t offset, size_t size);
		void PendingTransition(EBufferState dstState, Handle<Buffer> handle);
		
		std::vector<TextureStateTransition> FlushTextureTransitions() { return std::move(pendingTextureTransitions); }
		std::vector<BufferStateTransition> FlushBufferTransitions() { return std::move(pendingBufferTransitions); }

	private:
		//mutable std::mutex mutex;
		CacheRegistry& cacheRegistry;
		robin_hood::unordered_map<HandleUnderType, TextureState> textureStates;
		robin_hood::unordered_map<HandleUnderType, BufferState> bufferStates;

		std::vector<TextureStateTransition> pendingTextureTransitions;
		std::vector<BufferStateTransition> pendingBufferTransitions;

	};
}