#pragma once
#include <PCH.h>

namespace sy
{
	using Placement = size_t;
	static constexpr Placement InvalidPlacement = std::numeric_limits<Placement>::max();

	template <typename T>
	class HandleMap
	{
	public:
		class Handle
		{
		public:
			/** Invalid Handle<Empty Handle>/Not owned Handle */
			Handle() noexcept
				: owner(std::nullopt), placement(InvalidPlacement)
			{
			}

			Handle(const Handle& other) = default;

			Handle(Handle&& other) noexcept
				: owner(std::exchange(other.owner, std::nullopt)), placement(std::exchange(other.placement, InvalidPlacement))
			{
			}

			~Handle() = default;

			Handle& operator=(const Handle& rhs) = default;

			Handle& operator=(Handle&& rhs) noexcept
			{
				this->owner = std::exchange(rhs.owner, std::nullopt);
				this->placement = std::exchange(rhs.placement, InvalidPlacement);
				return *this;
			}

			[[nodiscard]] explicit operator bool() const
			{
				return IsValid();
			}

			[[nodiscard]] RefOptional<T> TryGetObject()
			{
				if (!IsOwned())
				{
					return std::nullopt;
				}

				return owner->get().TryGetObject(this->placement);
			}

			[[nodiscard]] CRefOptional<T> TryGetObject() const
			{
				if (!IsOwned())
				{
					return std::nullopt;
				}

				return owner->get().TryGetObject(this->placement);
			}

			[[nodiscard]] T& operator*()
			{
				const auto opt = TryGetObject();
				SY_ASSERT(opt, "Trying to access invalid handle.");
				return opt.value().get();
			}

			[[nodiscard]] const T& operator*() const
			{
				const auto opt = TryGetObject();
				SY_ASSERT(opt, "Trying to access invalid handle.");
				return opt.value().get();
			}

			[[nodiscard]] T* operator->()
			{
				const auto opt = TryGetObject();
				SY_ASSERT(opt, "Trying to access invalid handle.");
				return opt != std::nullopt ? &(opt.value().get()) : nullptr;
			}

			[[nodiscard]] const T* operator->() const
			{
				const auto opt = TryGetObject();
				SY_ASSERT(opt, "Trying to access invalid handle.");
				return opt != std::nullopt ? &(opt.value().get()) : nullptr;
			}

			[[nodiscard]] bool IsValid() const
			{
				return TryGetObject() != std::nullopt;
			}

			[[nodiscard]] std::optional<std::string_view> TryGetAlias() const
			{
				if (!IsValid())
				{
					return std::nullopt;
				}

				return owner->get().TryGetAlias(this->placement);
			}

			[[nodiscard]] std::string_view GetAlias() const
			{
				return *TryGetAlias();
			}

			[[nodiscard]] bool HasAlias() const
			{
				return TryGetAlias() != std::nullopt;
			}

			/** The Alias is unique string identifier for handle. */
			void SetAlias(const std::string_view alias)
			{
				if (IsValid())
				{
					owner->get().SetAlias(this->placement, alias);
				}
			}

			void RemoveAlias()
			{
				if (HasAlias())
				{
					/** Remove Alias from the handle. */
					owner->get().RemoveAlias(this->placement);
				}
			}

			void DestroySelf()
			{
				if (IsValid())
				{
					owner->get().Destroy(this->placement);
					owner = std::nullopt;
					placement = InvalidPlacement;
				}
			}

			[[nodiscard]] Placement GetPlacement() const
			{
				return IsValid() ? placement : InvalidPlacement;
			}

			[[nodiscard]] CRefOptional<HandleMap> GetOwner() const
			{
				return owner;
			}

		private:
			friend HandleMap;

			Handle(HandleMap& owner, const Placement placement)
				: owner(owner), placement(placement)
			{
			}

			[[nodiscard]] bool IsOwned() const noexcept
			{
				return owner != std::nullopt;
			}

		private:
			RefOptional<HandleMap> owner;
			Placement placement;
		};

	public:
		explicit HandleMap(const size_t reservedMapSize = 1024)
		{
			objectMap.reserve(reservedMapSize);
			objectMapIndexPool.reserve(reservedMapSize);
		}

		HandleMap(HandleMap&& other) noexcept
			: mutex(std::move(other.mutex)), handleCounter(std::swap(other.handleCounter, 0)), objectMap(std::move(other.objectMap)), objectMapIndexPool(std::move(other.objectMapIndexPool)), aliasToHandle(std::move(other.aliasToHandle)), handleToAlias(std::move(other.handleToAlias))
		{
		}

		~HandleMap() = default;

		/** Non-copyable */
		HandleMap(const HandleMap&) = delete;
		HandleMap& operator=(const HandleMap&) = delete;

		HandleMap& operator=(HandleMap&& rhs) noexcept
		{
			this->mutex = std::move(rhs.mutex);
			this->handleCounter = std::swap(rhs.handleCounter, 0);
			this->objectMap = std::move(rhs.objectMap);
			this->objectMapIndexPool = std::move(rhs.objectMapIndexPool);
			this->aliasToHandle = std::move(rhs.aliasToHandle);
			this->handleToAlias = std::move(rhs.handleToAlias);

			return *this;
		}

		[[nodiscard]] Handle Add(std::unique_ptr<T> object)
		{
			RWLock lock{ mutex };

			Placement placement = InvalidPlacement;
			if (!objectMapIndexPool.empty())
			{
				placement = objectMapIndexPool.back();
				objectMapIndexPool.pop_back();

				SY_ASSERT(placement < objectMap.size(), "The placement index must be lesser than size of object map.");
				SY_ASSERT(objectMap[placement] == nullptr, "Trying to place at already valid slot of object map!");
				objectMap[placement] = std::move(object);
			}
			else
			{
				placement = objectMap.size();
				objectMap.emplace_back(std::move(object));
			}

			return Handle{ *this, placement };
		}

		template <typename... Args>
		[[nodiscard]] Handle Add(Args&&... args)
		{
			return Add(std::make_unique<T>(std::forward<Args>(args)...));
		}

		[[nodiscard]] Handle QueryAlias(const std::string_view alias)
		{
			ReadOnlyLock lock{ mutex };
			const auto hashOfAlias = std::hash<std::string_view>()(alias);
			if (HasAliasUnsafe(alias) && HasValidAliasUnsafe(aliasToHandle[hashOfAlias]))
			{
				return Handle{ *this, aliasToHandle[hashOfAlias] };
			}

			return Handle{};
		}

		[[nodiscard]] Handle Query(const Placement placement)
		{
			ReadOnlyLock lock{ mutex };
			return ContainsUnsafe(placement) ? Handle{ *this, placement } : Handle{};
		}

	private:
		[[nodiscard]] bool ContainsUnsafe(const Placement placement) const
		{
			return (placement < objectMap.size()) && (objectMap[placement] != nullptr);
		}

		[[nodiscard]] bool HasAliasUnsafe(const std::string_view alias) const
		{
			return aliasToHandle.contains(std::hash<std::string_view>()(alias));
		}

		[[nodiscard]] bool HasAliasForPlacementUnsafe(const Placement placement) const
		{
			return handleToAlias.contains(placement);
		}

		[[nodiscard]] bool HasValidAliasUnsafe(const Placement placement) const
		{
			return ContainsUnsafe(placement) && HasAliasForPlacementUnsafe(placement) && HasAliasUnsafe(handleToAlias.find(placement)->second);
		}

		void SetAliasUnsafe(const Placement placement, const std::string_view newAlias)
		{
			const bool bContains = ContainsUnsafe(placement);
			const bool bIsExistAnyAliasForPlacement = HasAliasForPlacementUnsafe(placement);
			const bool bIsNewAliasAlreadyUsed = HasAliasUnsafe(newAlias);
			if (bContains && bIsExistAnyAliasForPlacement && !bIsNewAliasAlreadyUsed)
			{
				aliasToHandle.erase(std::hash<std::string_view>()(handleToAlias[placement]));
			}

			if (bContains && !bIsNewAliasAlreadyUsed)
			{
				aliasToHandle[std::hash<std::string_view>()(newAlias)] = placement;
				handleToAlias[placement] = newAlias;
			}
		}

		void RemoveAliasUnsafe(const Placement placement)
		{
			if (HasAliasForPlacementUnsafe(placement))
			{
				aliasToHandle.erase(std::hash<std::string_view>()(handleToAlias[placement]));
				handleToAlias.erase(placement);
			}
		}

		void SetAlias(const Placement placement, const std::string_view newAlias)
		{
			RWLock lock{ mutex };
			SetAliasUnsafe(placement, newAlias);
		}

		[[nodiscard]] RefOptional<T> TryGetObject(const Placement placement)
		{
			ReadOnlyLock lock{ mutex };
			if (!ContainsUnsafe(placement))
			{
				return std::nullopt;
			}

			return *objectMap[placement];
		}

		[[nodiscard]] CRefOptional<T> TryGetObject(const Placement placement) const
		{
			ReadOnlyLock lock{ mutex };
			if (!ContainsUnsafe(placement))
			{
				return std::nullopt;
			}

			return *objectMap[placement];
		}

		void RemoveAlias(const Placement placement)
		{
			RWLock lock{ mutex };
			RemoveAliasUnsafe(placement);
		}

		[[nodiscard]] std::optional<std::string_view> TryGetAlias(const Placement placement) const
		{
			ReadOnlyLock lock{ mutex };
			if (HasValidAliasUnsafe(placement))
			{
				return handleToAlias.find(placement)->second;
			}

			return std::nullopt;
		}

		void Destroy(const Placement placement)
		{
			RWLock lock{ mutex };
			if (ContainsUnsafe(placement))
			{
				RemoveAliasUnsafe(placement);
				objectMap[placement].reset();
				objectMapIndexPool.emplace_back(placement);
			}
		}

	private:
		mutable std::shared_mutex mutex;
		Placement handleCounter = 0;
		/** Handle Value-Object Map */
		std::vector<std::unique_ptr<T>> objectMap;
		std::vector<Placement> objectMapIndexPool;

		/** Alias-Handle Value Map */
		robin_hood::unordered_map<size_t, Placement> aliasToHandle;
		/** Handle Value-Alias Map */
		robin_hood::unordered_map<Placement, std::string> handleToAlias;
	};

	template <typename T>
	using Handle = typename HandleMap<T>::Handle;

	class HandleManager
	{
	private:
		using UntypedHandleMap = std::pair<void*, std::function<void(void*)>>;

	public:
		HandleManager() = default;

		~HandleManager()
		{
			for (auto& element : table)
			{
				UntypedHandleMap& value = element.second;
				value.second(value.first);
			}
		}

		/** Non-copyable */
		HandleManager(const HandleManager&) = delete;
		HandleManager& operator=(const HandleManager&) = delete;
		/** Non-movable */
		HandleManager(HandleManager&&) noexcept = delete;
		HandleManager& operator=(HandleManager&&) noexcept = delete;

		template <typename T>
		HandleMap<T>& GetHandleMap()
		{
			constexpr auto typeHash = TypeHash<T>;
			if (!table.contains(typeHash))
			{
				static std::once_flag initFlag;
				std::call_once(initFlag, [&]() {
					RWLock lock{ mutex };
					table[typeHash] = std::make_pair(
						reinterpret_cast<void*>(new HandleMap<T>()),
						[](const void* ptr) {
							if (ptr != nullptr)
							{
								delete static_cast<const HandleMap<T>*>(ptr);
							}
						});
				});
			}

			ReadOnlyLock lock{ mutex };
			return *(static_cast<HandleMap<T>*>(table[typeHash].first));
		}

		/** Proxy for HandleMaps */
		template <typename T>
		[[nodiscard]] Handle<T> Add(std::unique_ptr<T> object)
		{
			auto& handleMap = GetHandleMap<T>();
			return handleMap.Add(std::move(object));
		}

		template <typename T, typename... Args>
		[[nodiscard]] Handle<T> Add(Args&&... args)
		{
			auto& handleMap = GetHandleMap<T>();
			return handleMap.Add(std::forward<Args>(args)...);
		}

		template <typename T>
		[[nodiscard]] Handle<T> QueryAlias(const std::string_view alias)
		{
			auto& handleMap = GetHandleMap<T>();
			return handleMap.QueryAlias(alias);
		}

		template <typename T>
		[[nodiscard]] Handle<T> Query(const typename Placement placement)
		{
			auto& handleMap = GetHandleMap<T>();
			return handleMap.Query(placement);
		}

	private:
		mutable std::shared_mutex mutex;
		mutable robin_hood::unordered_map<TypeHashType, UntypedHandleMap> table;
	};
} // namespace sy
