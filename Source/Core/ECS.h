#pragma once
#include <PCH.h>

namespace sy::utils
{
/**
	 * ELF Hash function
	 * https://www.partow.net/programming/hashfunctions/index.html#StringHashing
	 */
constexpr uint32_t ELFHash(const char* str)
{
    unsigned int hash = 0;
    unsigned int x    = 0;
    for (unsigned int idx = 0; idx < sizeof(str); ++idx)
    {
        hash = (hash << 4) + (*str);
        x    = hash & 0xF0000000L;
        if (x != 0)
        {
            hash ^= (x >> 24);
        }

        hash &= ~x;
    }

    return hash;
}

inline size_t AlignForwardAdjustment(const size_t offset, size_t alignment) noexcept
{
    const size_t adjustment = alignment - (offset & (alignment - 1));
    if (adjustment == alignment)
    {
        return 0;
    }

    return adjustment;
}
} // namespace sy::utils

namespace sy::ecs
{
namespace views  = std::views;
namespace ranges = std::ranges;
} // namespace sy::ecs

#define SY_ECS_THREAD_SAFE false

namespace sy::ecs
{
struct Component
{
    virtual ~Component() = default;
};

template <typename T>
concept ComponentType = std::is_base_of_v<Component, T>;

enum class Entity : uint64_t
{
};

constexpr Entity INVALID_ENTITY_HANDLE            = static_cast<Entity>(0);
constexpr bool   USE_RANDOM_NUM_FOR_ENTITY_HANDLE = false;

inline Entity GenerateEntity()
{
    using EntityUnderlyingType = std::underlying_type_t<Entity>;
    if constexpr (USE_RANDOM_NUM_FOR_ENTITY_HANDLE)
    {
        static thread_local std::mt19937_64 generator(
            std::hash<std::thread::id>{}(std::this_thread::get_id()));

        std::uniform_int_distribution<EntityUnderlyingType> dist(std::numeric_limits<EntityUnderlyingType>::min() + 1, std::numeric_limits<EntityUnderlyingType>::max());
        return static_cast<Entity>(dist(generator));
    }

    static std::atomic<EntityUnderlyingType> handle = 1;
    return static_cast<Entity>(handle++);
}

using ComponentID                          = uint32_t;
constexpr ComponentID INVALID_COMPONENT_ID = 0;

template <ComponentType T>
constexpr ComponentID QueryComponentID()
{
    return INVALID_COMPONENT_ID;
}

template <ComponentType T>
constexpr ComponentID QueryComponentID(const T&)
{
    return QueryComponentID<T>;
}

struct ComponentInfo
{
    ComponentID ID = INVALID_COMPONENT_ID;
    std::string Name;
    size_t      Size      = 0;
    size_t      Alignment = 1;

    template <typename T>
    static ComponentInfo Generate()
    {
        ComponentInfo result{
            .ID        = QueryComponentID<T>(),
            .Name      = typeid(T).name(),
            .Size      = sizeof(T),
            .Alignment = alignof(T)};

        return result;
    }
};

// https://forum.unity.com/threads/is-it-guaranteed-that-random-access-within-a-16kb-chunk-will-not-cause-cache-miss.709940/
constexpr size_t DEFAULT_CHUNK_SIZE = 16384;
// https://stackoverflow.com/questions/34860366/why-buffers-should-be-aligned-on-64-byte-boundary-for-best-performance
constexpr size_t CACHE_LINE = 64;

struct ComponentRange
{
    size_t Offset = 0;
    size_t Size   = 0;

    static void ComponentCopy(void* destBaseAddress, void* srcBaseAddress, size_t destComponentIdx, size_t srcComponentIdx, ComponentRange destRange, ComponentRange srcRange) noexcept
    {
        assert(destRange.Size == srcRange.Size);
        void*       dest = (void*)((uintptr_t)destBaseAddress + destRange.Offset + (destComponentIdx * destRange.Size));
        const void* src  = (void*)((uintptr_t)srcBaseAddress + srcRange.Offset + (srcComponentIdx * srcRange.Size));
        std::memcpy(dest, src, srcRange.Size);
    }

    static void* ComponentAddress(void* baseAddress, size_t componentIdx, ComponentRange range) noexcept
    {
        return (void*)((uintptr_t)baseAddress + range.Offset + (componentIdx * range.Size));
    }
};

class Chunk
{
    using PoolType = std::priority_queue<size_t, std::vector<size_t>, std::greater<size_t>>;

public:
    Chunk(const size_t maxNumOfAllocations) :
        mem(_aligned_malloc(DEFAULT_CHUNK_SIZE, CACHE_LINE)), allocationPool({}), maxNumOfAllocations(maxNumOfAllocations)
    {
        for (size_t allocationIndex = 0; allocationIndex < MaxNumOfAllocations(); ++allocationIndex)
        {
            allocationPool.push(allocationIndex);
        }
    }

    Chunk(Chunk&& rhs) noexcept
        :
        mem(std::exchange(rhs.mem, nullptr)), allocationPool(std::move(rhs.allocationPool)), maxNumOfAllocations(std::exchange(rhs.maxNumOfAllocations, 0))
    {
    }

    ~Chunk()
    {
        if (mem != nullptr)
        {
            _aligned_free(mem);
            mem = nullptr;
        }
    }

    Chunk(const Chunk&)            = delete;
    Chunk& operator=(const Chunk&) = delete;

    Chunk& operator=(Chunk&& rhs) noexcept
    {
        mem                 = std::exchange(rhs.mem, nullptr);
        allocationPool      = std::move(rhs.allocationPool);
        maxNumOfAllocations = std::exchange(rhs.maxNumOfAllocations, 0);
        return (*this);
    }

    /** Return index of allocation */
    size_t Allocate()
    {
        assert(!IsFull());
        const size_t alloc = allocationPool.top();
        allocationPool.pop();
        return alloc;
    }

    /** Return moved allocation, only if return 0 when numOfAllocation is 1 */
    void Deallocate(size_t at)
    {
        assert(at < MaxNumOfAllocations());
        allocationPool.push(at);
    }

    [[nodiscard]] void* BaseAddress() const noexcept
    {
        return mem;
    }

    [[nodiscard]] bool IsEmpty() const noexcept
    {
        return allocationPool.size() == MaxNumOfAllocations();
    }

    [[nodiscard]] bool IsFull() const noexcept
    {
        return allocationPool.empty();
    }

    [[nodiscard]] size_t MaxNumOfAllocations() const noexcept
    {
        return maxNumOfAllocations;
    }

    [[nodiscard]] size_t NumOfAllocations() const noexcept
    {
        return MaxNumOfAllocations() - allocationPool.size();
    }

private:
    void*    mem;
    PoolType allocationPool;
    size_t   maxNumOfAllocations;
};

class ChunkList
{
public:
    struct Allocation
    {
        size_t ChunkIndex              = std::numeric_limits<size_t>::max();
        size_t AllocationIndexOfEntity = std::numeric_limits<size_t>::max();

        [[nodiscard]] bool IsFailedToAllocate() const noexcept
        {
            return (ChunkIndex == static_cast<size_t>(-1)) || (AllocationIndexOfEntity == static_cast<size_t>(-1));
        }
    };

    struct ComponentAllocationInfo
    {
        ComponentRange Range;
        ComponentID    ID = INVALID_COMPONENT_ID;
    };

public:
    ChunkList(const std::vector<ComponentInfo>& componentInfos)
    {
        size_t offset = 0;
        if (!componentInfos.empty())
        {
            for (const ComponentInfo& info : componentInfos)
            {
                componentAllocInfos.emplace_back(ComponentAllocationInfo{
                    .Range = ComponentRange{
                        .Offset = offset,
                        .Size   = info.Size},
                    .ID = info.ID});
                offset += info.Size;
            }

            componentAllocInfos.shrink_to_fit();
        }

        sizeOfData = offset;
        // assume component offsets are aligned as cache line. then calculate maximum align adjustment[1, CACHE_LINE-1](Not a optimal)
        // @TODO	Optimal alignment memory reservation.
        const size_t actualUsableChunkSize = (DEFAULT_CHUNK_SIZE - ((componentAllocInfos.size() - 1) * (CACHE_LINE - 1)));
        maxNumOfAllocationsPerChunk        = offset == 0 ? 0 : (actualUsableChunkSize / sizeOfData);

        for (size_t idx = 1; idx < componentAllocInfos.size(); ++idx)
        {
            const auto& beforeAllocInfo = componentAllocInfos.at(idx - 1);
            auto&       allocInfo       = componentAllocInfos.at(idx);

            allocInfo.Range.Offset = beforeAllocInfo.Range.Offset + (maxNumOfAllocationsPerChunk * beforeAllocInfo.Range.Size);
            allocInfo.Range.Offset += utils::AlignForwardAdjustment(allocInfo.Range.Offset, CACHE_LINE);
        }
    }

    ChunkList(ChunkList&& rhs) noexcept
        :
        chunks(std::move(rhs.chunks)), componentAllocInfos(std::move(rhs.componentAllocInfos)), sizeOfData(rhs.sizeOfData), maxNumOfAllocationsPerChunk(rhs.maxNumOfAllocationsPerChunk)
    {
    }

    ~ChunkList() = default;

    ChunkList(const ChunkList&)            = delete;
    ChunkList& operator=(const ChunkList&) = delete;

    ChunkList& operator=(ChunkList&& rhs) noexcept
    {
        chunks                      = std::move(rhs.chunks);
        componentAllocInfos         = std::move(rhs.componentAllocInfos);
        sizeOfData                  = rhs.sizeOfData;
        maxNumOfAllocationsPerChunk = rhs.maxNumOfAllocationsPerChunk;
        return (*this);
    }

    /** It doesn't call anyof constructor. */
    Allocation Create()
    {
        assert(sizeOfData > 0);
        const size_t freeChunkIndex = FreeChunkIndex();
        if (const bool bDoesNotFoundFreeChunk = freeChunkIndex >= chunks.size(); bDoesNotFoundFreeChunk)
        {
            chunks.emplace_back(maxNumOfAllocationsPerChunk);
        }

        Chunk&       chunk      = chunks.at(freeChunkIndex);
        const size_t allocIndex = chunk.Allocate();

        return Allocation{
            .ChunkIndex              = freeChunkIndex,
            .AllocationIndexOfEntity = allocIndex};
    }

    /** It does'nt call any destructor. */
    void Destroy(const Allocation allocation)
    {
        assert(!allocation.IsFailedToAllocate());
        assert(allocation.ChunkIndex < chunks.size());
        chunks.at(allocation.ChunkIndex).Deallocate(allocation.AllocationIndexOfEntity);
    }

    ComponentAllocationInfo AllocationInfoOfComponent(const ComponentID componentID) const
    {
        auto found = std::find_if(componentAllocInfos.cbegin(), componentAllocInfos.cend(),
                                  [componentID](const ComponentAllocationInfo& info) {
                                      return componentID == info.ID;
                                  });

        return (*found);
    }

    [[nodiscard]] bool Support(const ComponentID componentID) const
    {
        const auto found = std::find_if(componentAllocInfos.cbegin(), componentAllocInfos.cend(),
                                        [componentID](const ComponentAllocationInfo& info) {
                                            return componentID == info.ID;
                                        });

        return found != componentAllocInfos.cend();
    }

    [[nodiscard]] void* BaseAddressOf(const Allocation allocation) const
    {
        void* baseAddress = chunks.at(allocation.ChunkIndex).BaseAddress();
        return baseAddress;
    }

    [[nodiscard]] void* AddressOf(const Allocation allocation, const ComponentID componentID) const
    {
        const bool bIsValidChunkIndex = allocation.ChunkIndex < chunks.size();
        assert(bIsValidChunkIndex);

        if (Support(componentID))
        {
            const auto componentAllocInfo = AllocationInfoOfComponent(componentID);
            void*      baseAddress        = chunks.at(allocation.ChunkIndex).BaseAddress();

            return ComponentRange::ComponentAddress(baseAddress, allocation.AllocationIndexOfEntity,
                                                    componentAllocInfo.Range);
        }

        return nullptr;
    }

    [[nodiscard]] bool IsChunkFull(const size_t chunkIndex) const noexcept
    {
        assert(chunkIndex < chunks.size());
        return chunks.at(chunkIndex).IsFull();
    }

    [[nodiscard]] size_t FreeChunkIndex() const noexcept
    {
        size_t freeChunkIndex = 0;
        for (; freeChunkIndex < chunks.size(); ++freeChunkIndex)
        {
            if (!chunks.at(freeChunkIndex).IsFull())
            {
                break;
            }
        }

        return freeChunkIndex;
    }

    size_t ShrinkToFit()
    {
        // Erase-Remove idiom
        const auto reduced = std::erase_if(chunks, [](Chunk& chunk) noexcept {
            return chunk.IsEmpty();
        });

        chunks.shrink_to_fit();
        return reduced;
    }

    /** Just memory data copy, it never call any constructor or destructor. */
    static void MoveData(ChunkList& srcChunkList, const Allocation srcAllocation, const ChunkList& destChunkList, const Allocation destAllocation)
    {
        bool bIsValid = !srcAllocation.IsFailedToAllocate() && !destAllocation.IsFailedToAllocate();
        assert(bIsValid);

        bIsValid = bIsValid && (srcAllocation.ChunkIndex < srcChunkList.chunks.size() && destAllocation.ChunkIndex < destChunkList.chunks.size());
        assert(bIsValid);

        if (bIsValid)
        {
            void*       srcAddress              = srcChunkList.BaseAddressOf(srcAllocation);
            void*       destAddress             = destChunkList.BaseAddressOf(destAllocation);
            const auto& srcComponentAllocInfos  = srcChunkList.componentAllocInfos;
            const auto& destComponentAllocInfos = destChunkList.componentAllocInfos;
            for (const auto& srcComponentAllocInfo : srcComponentAllocInfos)
            {
                for (const auto& destComponentAllocInfo : destComponentAllocInfos)
                {
                    if (srcComponentAllocInfo.ID == destComponentAllocInfo.ID)
                    {
                        ComponentRange::ComponentCopy(destAddress, srcAddress,
                                                      destAllocation.AllocationIndexOfEntity,
                                                      srcAllocation.AllocationIndexOfEntity,
                                                      destComponentAllocInfo.Range, srcComponentAllocInfo.Range);
                    }
                }
            }

            srcChunkList.Destroy(srcAllocation);
        }
    }

private:
    std::vector<Chunk>                   chunks;
    std::vector<ComponentAllocationInfo> componentAllocInfos;
    size_t                               sizeOfData;
    size_t                               maxNumOfAllocationsPerChunk;
};

using Archetype = std::set<ComponentID>;

/**
	 * @brief	ComponentArchive itself guarantee thread-safety when SY_ECS_THREAD_SAFE is true. But write to component data which stored inside of chunk is not a thread-safe.
	 */
class ComponentArchive
{
public:
    struct DynamicComponentData
    {
        ComponentInfo              Info;
        std::function<void(void*)> DefaultConstructor;
        std::function<void(void*)> Destructor;
    };

    struct ArchetypeData
    {
        size_t                ArchetypeIndex = 0;
        ChunkList::Allocation Allocation;
    };

    template <ComponentType T>
    class ComponentHandle
    {
    public:
        ComponentHandle(const ComponentArchive& archive, const Entity entity) noexcept
            :
            archive(archive), entity(entity)
        {
        }

        ~ComponentHandle() = default;

        ComponentHandle(const ComponentHandle&) noexcept            = default;
        ComponentHandle(ComponentHandle&&) noexcept                 = default;
        ComponentHandle& operator=(const ComponentHandle&) noexcept = default;
        ComponentHandle& operator=(ComponentHandle&&) noexcept      = default;

        T& operator*()
        {
            return Reference();
        }

        const T& operator*() const
        {
            return Reference();
        }

        T* operator->()
        {
            return archive.Get<T>(entity);
        }

        const T* operator->() const
        {
            return archive.Get<T>(entity);
        }

        T& Reference()
        {
            return *archive.Get<T>(entity);
        }

        const T& Reference() const
        {
            return *archive.Get<T>(entity);
        }

        [[nodiscard]] Entity Owner() const noexcept
        {
            return entity;
        }

        [[nodiscard]] bool IsValid() const noexcept
        {
            return archive.Contains<T>(entity);
        }

        [[nodiscard]] constexpr ComponentID ID() const noexcept
        {
            return QueryComponentID<T>();
        }

    private:
        const ComponentArchive& archive;
        const Entity            entity;
    };

#if SY_ECS_THREAD_SAFE
    using Mutex_t        = std::shared_timed_mutex;
    using WriteLock_t    = std::unique_lock<Mutex_t>;
    using ReadOnlyLock_t = std::shared_lock<Mutex_t>;
#endif

public:
    ComponentArchive(const ComponentArchive&)            = delete;
    ComponentArchive(ComponentArchive&&)                 = delete;
    ComponentArchive& operator=(const ComponentArchive&) = delete;
    ComponentArchive& operator=(ComponentArchive&&)      = delete;

    ~ComponentArchive() noexcept(false)
    {
        std::vector<Entity> remainEntities;
        remainEntities.reserve(archetypeLUT.size());
        for (auto& entityArchetypePair : archetypeLUT)
        {
            remainEntities.emplace_back(entityArchetypePair.first);
        }

        for (const Entity entity : remainEntities)
        {
            Destroy(entity);
        }
    }

    static ComponentArchive& Instance()
    {
        std::call_once(instanceCreationOnceFlag, []() {
            instance.reset(new ComponentArchive());
        });

        return *instance;
    }

    static void DestroyInstance()
    {
        std::call_once(instanceDestructionOnceFlag, []() {
            delete instance.release();
        });
    }

    template <ComponentType T>
    void Archive()
    {
        dynamicComponentDataLUT[QueryComponentID<T>()] = DynamicComponentData{
            .Info               = ComponentInfo::Generate<T>(),
            .DefaultConstructor = [](void* ptr) { new (ptr) T(); },
            .Destructor         = [](void* ptr) {
                reinterpret_cast<T*>(ptr)->~T();
            }};
    }

    [[nodiscard]] bool Contains(const Entity entity, const ComponentID componentID) const
    {
#if SY_ECS_THREAD_SAFE
        ReadOnlyLock_t lock{mutex};
#endif
        const auto foundArchetypeItr = archetypeLUT.find(entity);
        if (foundArchetypeItr != archetypeLUT.end())
        {
            const auto& foundArchetypeData = (foundArchetypeItr->second);
            return ReferenceArchetype(foundArchetypeData.ArchetypeIndex).contains(componentID);
        }

        return false;
    }

    template <typename T>
    [[nodiscard]] bool Contains(const Entity entity) const
    {
        return Contains(entity, QueryComponentID<T>());
    }

    [[nodiscard]] bool IsSameArchetype(const Entity lhs, const Entity rhs) const
    {
#if SY_ECS_THREAD_SAFE
        ReadOnlyLock_t lock{mutex};
#endif
        const auto lhsItr = archetypeLUT.find(lhs);
        const auto rhsItr = archetypeLUT.find(rhs);
        if (lhsItr != archetypeLUT.end() && rhsItr != archetypeLUT.end())
        {
            const auto& lhsArchetypeData = lhsItr->second;
            const auto& rhsArchetypeData = rhsItr->second;
            return lhsArchetypeData.ArchetypeIndex == rhsArchetypeData.ArchetypeIndex;
        }

        // If both itrerator is entityLUT.end(), it means those are empty and at same time equal archetype.
        return lhsItr == rhsItr;
    }

    [[nodiscard]] Archetype QueryArchetype(const Entity entity) const
    {
#if SY_ECS_THREAD_SAFE
        ReadOnlyLock_t lock{mutex};
#endif
        if (archetypeLUT.contains(entity))
        {
            return ReferenceArchetype(archetypeLUT.find(entity)->second.ArchetypeIndex);
        }

        return Archetype();
    }

    /** Return nullptr, if component is already exist or failed to attach. */
    bool Attach(const Entity entity, const ComponentID componentID, const bool bCallDefaultConstructor = true)
    {
#if SY_ECS_THREAD_SAFE
        WriteLock_t lock{mutex};
#endif
        Component* result = nullptr;
        if (!ContainsUnsafe(entity, componentID))
        {
            if (!archetypeLUT.contains(entity))
            {
                archetypeLUT[entity] = ArchetypeData();
            }

            ArchetypeData& archetypeData = archetypeLUT[entity];
            Archetype      archetype     = ReferenceArchetype(archetypeData.ArchetypeIndex);
            archetype.insert(componentID);

            const auto                  newChunkListIdx = FindOrCreateChunkList(archetype);
            const ChunkList::Allocation newAllocation   = ReferenceChunkList(newChunkListIdx).Create();
            if (!newAllocation.IsFailedToAllocate() && !ReferenceArchetype(archetypeData.ArchetypeIndex).empty())
            {
                const auto oldChunkListIdx =
                    FindOrCreateChunkList(ReferenceArchetype(archetypeData.ArchetypeIndex));
                ChunkList::MoveData(
                    ReferenceChunkList(oldChunkListIdx), archetypeLUT[entity].Allocation,
                    ReferenceChunkList(newChunkListIdx), newAllocation);
            }

            archetypeData.Allocation     = newAllocation;
            archetypeData.ArchetypeIndex = newChunkListIdx;

            result = static_cast<Component*>(ReferenceChunkList(newChunkListIdx).AddressOf(newAllocation, componentID));
            if (result != nullptr && bCallDefaultConstructor)
            {
                const DynamicComponentData& dynamicComponentData = dynamicComponentDataLUT[componentID];
                dynamicComponentData.DefaultConstructor(result);
            }
        }

        return result != nullptr;
    }

    template <ComponentType T, typename... Args>
    bool Attach(const Entity entity, Args&&... args)
    {
        constexpr bool        bShouldCallDefaultConstructor = (sizeof...(Args) == 0);
        constexpr ComponentID componentID                   = QueryComponentID<T>();
        Component*            result                        = nullptr;

#if SY_ECS_THREAD_SAFE
        WriteLock_t lock{mutex};
#endif
        if (!ContainsUnsafe(entity, componentID))
        {
            if (!archetypeLUT.contains(entity))
            {
                archetypeLUT[entity] = ArchetypeData();
            }

            ArchetypeData& archetypeData = archetypeLUT[entity];
            Archetype      archetype     = ReferenceArchetype(archetypeData.ArchetypeIndex);
            archetype.insert(componentID);

            const auto                  newChunkListIdx = FindOrCreateChunkList(archetype);
            const ChunkList::Allocation newAllocation   = ReferenceChunkList(newChunkListIdx).Create();
            if (!newAllocation.IsFailedToAllocate() && !ReferenceArchetype(archetypeData.ArchetypeIndex).empty())
            {
                const auto oldChunkListIdx =
                    FindOrCreateChunkList(ReferenceArchetype(archetypeData.ArchetypeIndex));
                ChunkList::MoveData(
                    ReferenceChunkList(oldChunkListIdx), archetypeLUT[entity].Allocation,
                    ReferenceChunkList(newChunkListIdx), newAllocation);
            }

            archetypeData.Allocation     = newAllocation;
            archetypeData.ArchetypeIndex = newChunkListIdx;

            result = static_cast<Component*>(ReferenceChunkList(newChunkListIdx).AddressOf(newAllocation, componentID));
            if (result != nullptr)
            {
                if (bShouldCallDefaultConstructor)
                {
                    const DynamicComponentData& dynamicComponentData = dynamicComponentDataLUT[componentID];
                    dynamicComponentData.DefaultConstructor(result);
                }
                else
                {
                    new (result) T(std::forward<Args>(args)...);
                }
            }
        }

        return result != nullptr;
    }

    void Detach(const Entity entity, const ComponentID componentID)
    {
#if SY_ECS_THREAD_SAFE
        WriteLock_t lock{mutex};
#endif
        if (ContainsUnsafe(entity, componentID))
        {
            ArchetypeData& archetypeData = archetypeLUT[entity];
            Archetype      archetype     = ReferenceArchetype(archetypeData.ArchetypeIndex);
            archetype.erase(componentID);

            const auto                  oldChunkListIdx    = FindOrCreateChunkList(ReferenceArchetype(archetypeData.ArchetypeIndex));
            const ChunkList::Allocation oldAllocation      = archetypeData.Allocation;
            void*                       detachComponentPtr = ReferenceChunkList(oldChunkListIdx).AddressOf(oldAllocation, componentID);

            const DynamicComponentData& dynamicComponentData = dynamicComponentDataLUT[componentID];
            dynamicComponentData.Destructor(detachComponentPtr);

            if (!archetype.empty())
            {
                const auto                  newChunkListIdx = FindOrCreateChunkList(archetype);
                const ChunkList::Allocation newAllocation   = ReferenceChunkList(newChunkListIdx).Create();
                ChunkList::MoveData(
                    ReferenceChunkList(oldChunkListIdx), oldAllocation,
                    ReferenceChunkList(newChunkListIdx), newAllocation);
                archetypeData.Allocation     = newAllocation;
                archetypeData.ArchetypeIndex = newChunkListIdx;
            }
            else
            {
                ReferenceChunkList(oldChunkListIdx).Destroy(oldAllocation);
                archetypeData = ArchetypeData();
            }
        }
    }

    template <ComponentType T>
    void Detach(const Entity entity)
    {
        Detach(entity, QueryComponentID<T>());
    }

    /**
		 * @brief	Return Deferred Access Handle Object.
		 */
    template <ComponentType T>
    [[nodiscard]] ComponentHandle<T> GetHandle(const Entity entity) const noexcept
    {
        return ComponentHandle<T>(*this, entity);
    }

    [[nodiscard]] Component* Get(const Entity entity, const ComponentID componentID) const
    {
#if SY_ECS_THREAD_SAFE
        ReadOnlyLock_t lock{mutex};
#endif
        if (ContainsUnsafe(entity, componentID))
        {
            const auto&                  archetypeData = archetypeLUT.find(entity)->second;
            const Archetype&             archetype     = ReferenceArchetype(archetypeData.ArchetypeIndex);
            const ChunkList::Allocation& allocation    = archetypeData.Allocation;
            for (size_t idx = 1; idx < chunkListLUT.size(); ++idx) // Except null archetype
            {
                if (archetypeData.ArchetypeIndex == idx)
                {
                    const auto& chunkList = chunkListLUT.at(idx).second;
                    return static_cast<Component*>(chunkList.AddressOf(allocation, componentID));
                }
            }
        }

        return nullptr;
    }

    template <ComponentType T>
    [[nodiscard]] T* Get(const Entity entity) const
    {
        return reinterpret_cast<T*>(Get(entity, QueryComponentID<T>()));
    }

    void Destroy(const Entity entity)
    {
#if SY_ECS_THREAD_SAFE
        WriteLock_t lock(mutex);
#endif
        if (archetypeLUT.contains(entity))
        {
            const auto&      archetypeData = archetypeLUT[entity];
            const Archetype& archetype     = ReferenceArchetype(archetypeData.ArchetypeIndex);
            if (!archetype.empty())
            {
                const auto                  chunkList     = FindOrCreateChunkList(archetype);
                const ChunkList::Allocation oldAllocation = archetypeData.Allocation;
                for (const ComponentID componentID : archetype)
                {
                    void*                       detachComponentPtr   = ReferenceChunkList(chunkList).AddressOf(oldAllocation, componentID);
                    const DynamicComponentData& dynamicComponentData = dynamicComponentDataLUT[componentID];
                    dynamicComponentData.Destructor(detachComponentPtr);
                }

                ReferenceChunkList(chunkList).Destroy(oldAllocation);
            }

            archetypeLUT.erase(entity);
        }
    }

    /**
		 * Trying to de-fragment 'entire' chunk list and chunks(except not fragmented chunk which is full)
		 * It maybe will nullyfies any references, pointers that acquired from Attach and Get methods.
		 */
    void Defragmentation()
    {
#if SY_ECS_THREAD_SAFE
        WriteLock_t lock(mutex);
#endif
        for (auto& archetypeLUTPair : archetypeLUT)
        {
            const Entity     entity        = archetypeLUTPair.first;
            auto&            archetypeData = archetypeLUTPair.second;
            const Archetype& archetype     = ReferenceArchetype(archetypeData.ArchetypeIndex);
            if (!archetype.empty() && !archetypeData.Allocation.IsFailedToAllocate())
            {
                const size_t chunkListIdx = FindChunkList(archetype);
                if (chunkListIdx != chunkListLUT.size())
                {
                    ChunkList&   chunkListRef   = ReferenceChunkList(chunkListIdx);
                    const size_t freeChunkIndex = chunkListRef.FreeChunkIndex();
                    if (freeChunkIndex <= archetypeData.Allocation.ChunkIndex)
                    {
                        const ChunkList::Allocation newAllocation = chunkListRef.Create();
                        ChunkList::MoveData(
                            chunkListRef, archetypeData.Allocation,
                            chunkListRef, newAllocation);

                        archetypeData.Allocation = newAllocation;
                    }
                }
            }
        }
    }

    size_t ShrinkToFit(const bool bPerformShrinkAfterDefrag = true)
    {
        if (bPerformShrinkAfterDefrag)
        {
            Defragmentation();
        }

#if SY_ECS_THREAD_SAFE
        WriteLock_t lock(mutex);
#endif
        chunkListLUT.shrink_to_fit();

        size_t reduced = 0;
        for (auto& chunkList : chunkListLUT | views::values)
        {
            reduced += chunkList.ShrinkToFit();
        }

        return reduced;
    }

private:
    ComponentArchive() noexcept(false)
    {
        chunkListLUT.emplace_back(Archetype(), ChunkList({}));
    }

    size_t FindOrCreateChunkList(const Archetype& archetype)
    {
        size_t idx = 0;
        for (; idx < chunkListLUT.size(); ++idx)
        {
            if (chunkListLUT.at(idx).first == archetype)
            {
                break;
            }
        }

        if (idx == chunkListLUT.size())
        {
            chunkListLUT.emplace_back(archetype, ChunkList(RetrieveComponentInfosFromArchetype(archetype)));
        }

        return idx;
    }

    [[nodiscard]] size_t FindChunkList(const Archetype& archetype) const
    {
        size_t idx = 0;
        for (; idx < chunkListLUT.size(); ++idx)
        {
            if (chunkListLUT.at(idx).first == archetype)
            {
                return idx;
            }
        }

        return idx;
    }

    [[nodiscard]] bool ContainsUnsafe(const Entity entity, const ComponentID componentID) const
    {
        const auto foundArchetypeItr = archetypeLUT.find(entity);
        if (foundArchetypeItr != archetypeLUT.end())
        {
            const auto& foundArchetypeData = (foundArchetypeItr->second);
            return ReferenceArchetype(foundArchetypeData.ArchetypeIndex).contains(componentID);
        }

        return false;
    }

    /**
		 * To prevent vector re-allocations, always ref chunk list through this method.
		 * Do not reference ChunkList directly when exist possibility to chunkListLUT get modified.
		 */
    ChunkList& ReferenceChunkList(const size_t idx)
    {
        return chunkListLUT.at(idx).second;
    }

    [[nodiscard]] const Archetype& ReferenceArchetype(const size_t idx) const
    {
        return chunkListLUT.at(idx).first;
    }

    [[nodiscard]] std::vector<ComponentInfo> RetrieveComponentInfosFromArchetype(const Archetype& archetype) const
    {
        std::vector<ComponentInfo> res{};
        res.reserve(archetype.size());
        for (const ComponentID componentID : archetype)
        {
            const auto& foundComponentDynamicData = dynamicComponentDataLUT.find(componentID)->second;
            res.emplace_back(foundComponentDynamicData.Info);
        }

        return res;
    }

private:
    static inline std::unique_ptr<ComponentArchive> instance;
    static inline std::once_flag                    instanceCreationOnceFlag;
    static inline std::once_flag                    instanceDestructionOnceFlag;
#if SY_ECS_THREAD_SAFE
    mutable Mutex_t mutex;
#endif
    robin_hood::unordered_flat_map<ComponentID, DynamicComponentData> dynamicComponentDataLUT;
    robin_hood::unordered_flat_map<Entity, ArchetypeData>             archetypeLUT;
    std::vector<std::pair<Archetype, ChunkList>>                      chunkListLUT;
};

template <ComponentType T>
using ComponentHandle = ComponentArchive::ComponentHandle<T>;

namespace Filter
{
static std::vector<Entity> All(const ComponentArchive& archive, const std::vector<Entity>& entities, const Archetype& filter)
{
    std::vector<Entity> result;
    result.reserve((entities.size() / 2) + 2); /** Conservative reserve */

    for (const Entity entity : entities)
    {
        const Archetype& entityArchetype = archive.QueryArchetype(entity);
        if (!entityArchetype.empty() && std::includes(entityArchetype.cbegin(), entityArchetype.cend(), filter.cbegin(), filter.cend()))
        {
            result.push_back(entity);
        }
    }

    result.shrink_to_fit();
    return result;
}

static std::vector<Entity> Any(const ComponentArchive& archive, const std::vector<Entity>& entities, const Archetype& filter)
{
    assert(!filter.empty() && "Filter Archetype must contains at least one element.");
    std::vector<Entity> result;
    result.reserve((entities.size() / 2) + 2); /** Conservative reserve */

    for (const Entity entity : entities)
    {
        const Archetype& entityArchetype = archive.QueryArchetype(entity);
        if (!entityArchetype.empty())
        {
            Archetype intersection = {};
            std::set_intersection(
                filter.begin(), filter.end(),
                entityArchetype.begin(), entityArchetype.end(),
                std::inserter(intersection, intersection.end()));

            if (!intersection.empty())
            {
                result.push_back(entity);
            }
        }
    }

    result.shrink_to_fit();
    return result;
}

static std::vector<Entity> None(const ComponentArchive& archive, const std::vector<Entity>& entities, const Archetype& filter)
{
    assert(!filter.empty() && "Filter Archetype must contains at least one element.");
    std::vector<Entity> result;
    result.reserve((entities.size() / 2) + 2); /** Conservative reserve */

    for (const Entity entity : entities)
    {
        const Archetype& entityArchetype = archive.QueryArchetype(entity);
        if (!entityArchetype.empty())
        {
            Archetype intersection = {};
            std::set_intersection(
                filter.begin(), filter.end(),
                entityArchetype.begin(), entityArchetype.end(),
                std::inserter(intersection, intersection.end()));

            if (intersection.empty())
            {
                result.push_back(entity);
            }
        }
    }

    result.shrink_to_fit();
    return result;
}

template <ComponentType... Ts>
/** Entities which has all of given component types. */
std::vector<Entity> All(const ComponentArchive& archive, const std::vector<Entity>& entities)
{
    const Archetype filterArchetype = {QueryComponentID<Ts>()...};
    return All(archive, entities, filterArchetype);
}

template <ComponentType... Ts>
/** Entities which has any of given component types. */
std::vector<Entity> Any(const ComponentArchive& archive, const std::vector<Entity>& entities)
{
    const Archetype filterArchetype = {QueryComponentID<Ts>()...};
    return Any(archive, entities, filterArchetype);
}

template <ComponentType... Ts>
/** Entities which has none of given component types. */
std::vector<Entity> None(const ComponentArchive& archive, const std::vector<Entity>& entities)
{
    const Archetype filterArchetype = {QueryComponentID<Ts>()...};
    return None(archive, entities, filterArchetype);
}
} // namespace Filter
} // namespace sy::ecs

#define COMPONENT_TYPE_HASH(x) sy::utils::ELFHash(#x)

#define DeclareComponent(ComponentType)                                      \
    struct ComponentType##Registeration                                      \
    {                                                                        \
        ComponentType##Registeration()                                       \
        {                                                                    \
            auto& archive = sy::ComponentArchive::Instance();                \
            archive.Archive<ComponentType>();                                \
        }                                                                    \
                                                                             \
    private:                                                                 \
        static ComponentType##Registeration registeration;                   \
    };                                                                       \
    template <>                                                              \
    constexpr sy::ComponentID sy::QueryComponentID<ComponentType>()          \
    {                                                                        \
        constexpr uint32_t genID = COMPONENT_TYPE_HASH(ComponentType);       \
        static_assert(genID != 0 && "Generated Component ID is not valid."); \
        return static_cast<sy::ComponentID>(genID);                          \
    }
#define DefineComponent(ComponentType) ComponentType##Registeration ComponentType##Registeration::registeration;
