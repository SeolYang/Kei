#pragma once
#include <PCH.h>

namespace sy::vk
{
	inline auto ResolveTextureSubResourceRangeIndex(const size_t mipLevel, const size_t arrayLayer, const size_t numMipLevels)
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

		struct BufferState
		{
			Handle<Buffer> Handle = {};
			EBufferState State = EBufferState::None;
		};

		struct TextureStateTransition
		{
			const Texture& Target;
			ETextureState Before;
			ETextureState After;
			TextureSubResourceRange SubResourceRange;
		};

		struct BufferStateTransition
		{
			const Buffer& Target;
			EBufferState Before;
			EBufferState After;
		};

	public:
		ResourceStateTracker(HandleManager& handleManager);
		~ResourceStateTracker() override = default;

		void Register(Handle<Texture> handle);
		void UnRegister(Handle<Texture> handle);
		void Register(Handle<Buffer> handle);
		void UnRegister(Handle<Buffer> handle);

		void PendingTransition(ETextureState dstState, Handle<Texture> handle,
			std::span<const TextureSubResource> subResources = {});
		void PendingTransition(EBufferState dstState, Handle<Buffer> handle, size_t offset, size_t size);
		void PendingTransition(EBufferState dstState, Handle<Buffer> handle);

		std::vector<TextureStateTransition> FlushTextureTransitions()
		{
			return std::move(pendingTextureTransitions);
		}

		std::vector<BufferStateTransition> FlushBufferTransitions()
		{
			return std::move(pendingBufferTransitions);
		}

	private:
		HandleManager& handleManager;
		robin_hood::unordered_map<Placement, TextureState> textureStates;
		robin_hood::unordered_map<Placement, BufferState> bufferStates;

		std::vector<TextureStateTransition> pendingTextureTransitions;
		std::vector<BufferStateTransition> pendingBufferTransitions;
	};
} // namespace sy::vk
