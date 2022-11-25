#pragma once
#include <Core.h>

namespace sy
{
    template <typename SlotDataType = void>
    class Pool
    {
    public:
        template <typename DataType>
        struct SlotType
        {
            size_t Offset;
            DataType Data;
        };

        template <>
        struct SlotType<void>
        {
            size_t Offset;
        };

        using Slot_t = SlotType<SlotDataType>;

    public:
        Pool(const size_t sizePerSlot, const size_t numOfGrowSlots = 64) :
            sizePerSlot(sizePerSlot),
            numOfGrowSlots(numOfGrowSlots),
            allocatedSize(0)
        {
        }

        [[nodiscard]] Slot_t Allocate()
        {
            if (freeSlots.empty())
            {
                Grow();
            }

            Slot_t slot = std::move(freeSlots.front());
            freeSlots.pop();
            return slot;
        }

        void Deallocate(const Slot_t& slot)
        {
            freeSlots.emplace(slot);
        }

        [[nodiscard]] size_t GetAllocatedSize() const { return allocatedSize; }

    private:
        void Grow()
        {
            for (size_t idx = 0; idx < numOfGrowSlots; ++idx)
            {
                freeSlots.emplace(Slot_t{ allocatedSize });
                allocatedSize += sizePerSlot;
            }
        }

    private:
        std::queue<Slot_t> freeSlots;
        const size_t sizePerSlot;
        const size_t numOfGrowSlots;
        size_t allocatedSize;

    };

    using OffsetPool = Pool<void>;

}
