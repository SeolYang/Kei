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
	class ResourceCache : public NonCopyable
	{
		class CacheBase : public NonCopyable
		{
		};

		template <typename R>
		class Cache : public CacheBase
		{
		public:
			Handle<R> Add(std::unique_ptr<R> resource)
			{
				if (const auto key = reinterpret_cast<size_t>(resource.get()); cachedFlag.contains(key))
				{
					spdlog::warn("Trying to add duplicated resource to cache.");
					return { };
				}

				const auto newHandle = Handle<R>{ ++handleOffset };
				cached[newHandle.Value] = std::move(resource);
				return newHandle;
			}

			CRefOptional<R> Load(const Handle<R> handle) const
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

			RefOptional<R> Load(const Handle<R> handle)
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

				return Ref(*(found->second));
			}

		private:
			size_t handleOffset = 0;
			robin_hood::unordered_set<size_t> cachedFlag;
			robin_hood::unordered_map<size_t, std::unique_ptr<R>> cached;

		};

	public:
		template <typename R>
		RefOptional<R> Load(const Handle<R> handle)
		{
			Cache<R>& cache = AcquireOrCreate<R>();
			return cache.Load(handle);
		}

		template <typename R>
		CRefOptional<R> Load(const Handle<R> handle) const
		{
			CRefOptional<Cache<R>> cacheOpt = Acquire<R>();
			if (!cacheOpt)
			{
				return std::nullopt;
			}

			const Cache<R>& cache = Unwrap(cacheOpt);
			return cache.Load(handle);
		}

		template <typename R>
		Handle<R> Add(std::unique_ptr<R> ptr)
		{
			Cache<R>& cache = AcquireOrCreate<R>();
			return cache.Add(std::move(ptr));
		}

		void Clear()
		{
			caches.clear();
		}

	private:
		template <typename R>
		RefOptional<Cache<R>> Acquire()
		{
			const auto& type = typeid(R);
			const auto key = type.hash_code();
			if (!caches.contains(key))
			{
				return std::nullopt;
			}

			return static_cast<Cache<R>&>(*caches[key]);
		}

		template <typename R>
		CRefOptional<Cache<R>> Acquire() const
		{
			const auto& type = typeid(R);
			const auto key = type.hash_code();
			if (!caches.contains(key))
			{
				return std::nullopt;
			}

			return static_cast<const Cache<R>&>(*caches[key]);
		}

		template <typename R>
		Cache<R>& AcquireOrCreate()
		{
			if (const auto cacheOpt = Acquire<R>(); cacheOpt)
			{
				return Unwrap(cacheOpt);
			}

			const auto& type = typeid(R);
			const auto key = type.hash_code();
			caches[key] = std::make_unique<Cache<R>>();

			return static_cast<Cache<R>&>(*caches[key]);
		}

	private:
		robin_hood::unordered_map<size_t, std::unique_ptr<CacheBase>> caches;

	};
}