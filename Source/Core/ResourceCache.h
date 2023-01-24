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

			[[nodiscard]] CRefOptional<T> Load(const Handle<T> handle) const
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

			[[nodiscard]] RefOptional<T> Load(const Handle<T> handle)
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

			[[nodiscard]] auto Load(const std::string_view alias) const
			{
				return Load(QueryAlias(alias));
			}

			[[nodiscard]] auto Load(const std::string_view alias)
			{
				return Load(QueryAlias(alias));
			}

			void SetAlias(const std::string_view alias, const Handle<T> handle)
			{
				if (!Contains(alias) && Contains(handle))
				{
					const auto key = std::hash<std::string_view>()(alias);
					aliasMap[key] = handle.Value;
				}
			}

			[[nodiscard]] bool Contains(const Handle<T> handle) const
			{
				return cached.contains(handle.Value);
			}

			[[nodiscard]] bool Contains(const std::string_view alias) const
			{
				const auto key = std::hash<std::string_view>()(alias);
				const auto found = aliasMap.find(key);
				return (found != aliasMap.end()) && Contains(Handle<T>{found->second});
			}

			[[nodiscard]] Handle<T> QueryAlias(const std::string_view alias) const
			{
				const auto key = std::hash<std::string_view>()(alias);
				auto found = aliasMap.find(key);
				if (found != aliasMap.end())
				{
					return Handle<T>{ found->second };
				}

				return {};
			}

		private:
			size_t handleOffset = 0;
			robin_hood::unordered_set<size_t> cachedFlag;
			robin_hood::unordered_map<size_t, std::unique_ptr<T>> cached;
			robin_hood::unordered_map<size_t, size_t> aliasMap;

		};

	public:
		template <typename T>
		[[nodiscard]] RefOptional<T> Load(const Handle<T> handle)
		{
			Cache<T>& cache = AcquireOrCreate<T>();
			return cache.Load(handle);
		}

		template <typename T>
		[[nodiscard]] CRefOptional<T> Load(const Handle<T> handle) const
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
		[[nodiscard]] Handle<T> Add(std::unique_ptr<T> ptr)
		{
			Cache<T>& cache = AcquireOrCreate<T>();
			return cache.Add(std::move(ptr));
		}

		template <typename T, typename... Args>
		[[nodiscard]] Handle<T> Add(Args&&... args)
		{
			return Add(std::make_unique<T>(std::forward<Args>(args)...));
		}

		template <typename T>
		void SetAlias(const std::string_view alias, const Handle<T> handle)
		{
			Cache<T>& cache = AcquireOrCreate<T>();
			cache.SetAlias(alias, handle);
		}

		template <typename T>
		[[nodiscard]] bool Contains(const Handle<T> handle) const
		{
			auto cacheOpt = Acquire<T>();
			if (!cacheOpt)
			{
				return false;
			}

			return Unwrap(cacheOpt).Contains(handle);
		}

		template <typename T>
		[[nodiscard]] bool Contains(const std::string_view alias) const
		{
			auto cacheOpt = Acquire<T>();
			if (!cacheOpt)
			{
				return false;
			}

			return Unwrap(cacheOpt).Contains(alias);
		}

		template <typename T>
		[[nodiscard]] Handle<T> QueryAlias(const std::string_view alias) const
		{
			auto cacheOpt = Acquire<T>();
			if (!cacheOpt)
			{
				return {};
			}

			return Unwrap(cacheOpt).QueryAlias(alias);
		}

		template <typename T>
		[[nodiscard]] auto Load(const std::string_view alias) const
		{
			auto cacheOpt = Acquire<T>();
			if (!cacheOpt)
			{
				return std::nullopt;
			}

			return Unwrap(cacheOpt).Load(alias);
		}

		template <typename T>
		[[nodiscard]] auto Load(const std::string_view alias)
		{
			Cache<T>& cache = AcquireOrCreate<T>();
			return cache.Load(alias);
		}

		void Clear()
		{
			caches.clear();
		}

	private:
		template <typename T>
		CRefOptional<Cache<T>> Acquire() const
		{
			const auto& type = typeid(T);
			const auto key = type.hash_code();
			if (!caches.contains(key))
			{
				return std::nullopt;
			}

			const auto found = caches.find(key);
			return static_cast<const Cache<T>&>(*found->second);
		}

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