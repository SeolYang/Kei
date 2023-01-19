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

		template <typename T>
		class Cache : public CacheBase
		{
		public:
			Handle<T> Add(std::unique_ptr<T> resource)
			{
				if (const auto key = reinterpret_cast<size_t>(resource.get()); cachedFlag.contains(key))
				{
					spdlog::warn("Trying to add duplicated resource to cache.");
					return { };
				}

				const auto newHandle = Handle<T>{ ++handleOffset };
				cached[newHandle.Value] = std::move(resource);
				return newHandle;
			}

			CRefOptional<T> Load(const Handle<T> handle) const
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

			RefOptional<T> Load(const Handle<T> handle)
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
			robin_hood::unordered_map<size_t, std::unique_ptr<T>> cached;

		};

	public:
		template <typename T>
		RefOptional<T> Load(const Handle<T> handle)
		{
			Cache<T>& cache = AcquireOrCreate<T>();
			return cache.Load(handle);
		}

		template <typename T>
		CRefOptional<T> Load(const Handle<T> handle) const
		{
			CRefOptional<Cache<T>> cacheOpt = Acquire<T>();
			if (!cacheOpt)
			{
				return std::nullopt;
			}

			const Cache<T>& cache = Unwrap(cacheOpt);
			return cache.Load(handle);
		}

		template <typename T>
		Handle<T> Add(std::unique_ptr<T> ptr)
		{
			Cache<T>& cache = AcquireOrCreate<T>();
			return cache.Add(std::move(ptr));
		}

		template <typename T, typename... Args>
		Handle<T> Add(Args&&... args)
		{
			return Add(std::make_unique<T>(std::forward<Args>(args)...));
		}

		void Clear()
		{
			caches.clear();
		}

	private:
		template <typename T>
		RefOptional<Cache<T>> Acquire()
		{
			const auto& type = typeid(T);
			const auto key = type.hash_code();
			if (!caches.contains(key))
			{
				return std::nullopt;
			}

			return static_cast<Cache<T>&>(*caches[key]);
		}

		template <typename T>
		CRefOptional<Cache<T>> Acquire() const
		{
			const auto& type = typeid(T);
			const auto key = type.hash_code();
			if (!caches.contains(key))
			{
				return std::nullopt;
			}

			return static_cast<const Cache<T>&>(*caches[key]);
		}

		template <typename T>
		Cache<T>& AcquireOrCreate()
		{
			if (const auto cacheOpt = Acquire<T>(); cacheOpt)
			{
				return Unwrap(cacheOpt);
			}

			const auto& type = typeid(T);
			const auto key = type.hash_code();
			caches[key] = std::make_unique<Cache<T>>();

			return static_cast<Cache<T>&>(*caches[key]);
		}

	private:
		robin_hood::unordered_map<size_t, std::unique_ptr<CacheBase>> caches;

	};
}