#pragma once
#include <PCH.h>

namespace sy::vk
{
	class Texture;
	class Buffer;
}

namespace sy::render
{
	class Mesh;
}

namespace sy
{
	class CacheBase : public NonCopyable
	{
	};

	template <typename T>
	concept Named = std::derived_from<T, NamedType>;

	template <Named R>
	class Cache : public CacheBase
	{
	public:
		Handle<R> Add(std::unique_ptr<R> resource)
		{
			const auto key = std::hash<std::string>()(resource->GetName());
			if (cached.find(key) == cached.end())
			{
				cached[key] = std::move(resource);
			}
			else
			{
				spdlog::warn("Trying to add duplicated resource to cache.");
			}

			return { key };
		}

		std::optional<CRef<R>> Load(const Handle<R> handle) const
		{
			if (!handle)
			{
				return std::nullopt;
			}

			const auto found = cached.find(handle.Value);
			if (found == cached.end())
			{
				return std::nullopt;
			}

			return CRef(*(found->second));
		}

		std::optional<Ref<R>> Load(const Handle<R> handle)
		{
			const std::optional<CRef<R>> found = Load(handle);
			if (!found)
			{
				return std::nullopt;
			}

			return Ref(const_cast<R&>(found.value()()));
		}

	private:
		robin_hood::unordered_map<size_t, std::unique_ptr<R>> cached;

	};

	class CacheRegistry : public NonCopyable
	{
	public:
		template <Named R>
		Cache<R>& Acquire()
		{
			const auto& type = typeid(R);
			const size_t key = type.hash_code();
			const auto found = caches.find(key);
			if (found == caches.end())
			{
				std::unique_ptr<Cache<R>> newCache = std::make_unique<Cache<R>>();
				caches[key] = std::move(newCache);
			}

			return reinterpret_cast<Cache<R>&>(*caches[key]);
		}

	private:
		robin_hood::unordered_map<size_t, std::unique_ptr<CacheBase>> caches;

	};
}