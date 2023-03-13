#pragma once
#include <PCH.h>

namespace sy
{
	template <typename DataType>
	struct SlotType
	{
		size_t Offset;
		DataType Data;

		void Reset()
		{
			Offset = -1;
		}

		[[nodiscard]] bool IsValidOffset() const
		{
			return Offset != -1;
		}
	};

	template <>
	struct SlotType<void>
	{
		size_t Offset;

		void Reset()
		{
			Offset = -1;
		}

		[[nodiscard]] bool IsValidOffset() const
		{
			return Offset != -1;
		}
	};

	template <typename DataType>
	using SlotPtr = std::unique_ptr<SlotType<DataType>, std::function<void(const SlotType<DataType>*)>>;
	using OffsetSlotPtr = SlotPtr<void>;

	template <typename SlotDataType = void>
	class FixedPool
	{
	public:
		using Slot_t = SlotType<SlotDataType>;

	public:
		FixedPool(const size_t sizePerSlot = 1, size_t maxSlotCount = 0) :
			sizePerSlot(sizePerSlot),
			maxSlotCount(0),
			allocatedSize(0)
		{
			Grow(maxSlotCount);
		}

		[[nodiscard]] Slot_t Allocate()
		{
			Slot_t slot = std::move(freeSlots.front());
			freeSlots.pop();
			return std::move(slot);
		}

		void Deallocate(const Slot_t& slot)
		{
			freeSlots.emplace(slot);
		}

		[[nodiscard]] size_t GetAllocatedSize() const
		{
			return allocatedSize;
		}

		void Grow(const size_t additionalSlotCount)
		{
			if (additionalSlotCount > 0)
			{
				for (size_t idx = 0; idx < additionalSlotCount; ++idx)
				{
					freeSlots.emplace(allocatedSize);
					allocatedSize += sizePerSlot;
					++maxSlotCount;
				}
			}
		}

	private:
		std::queue<Slot_t> freeSlots;
		const size_t sizePerSlot;
		size_t maxSlotCount;
		size_t allocatedSize;
	};

	using FixedOffsetPool = FixedPool<void>;

	template <typename SlotDataType = void>
	class Pool
	{
	public:
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

		[[nodiscard]] size_t GetAllocatedSize() const
		{
			return allocatedSize;
		}

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
