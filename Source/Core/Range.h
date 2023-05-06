#pragma once
#include <optional>

namespace sy
{
template <typename T>
struct Range
{
public:
    template <typename C>
    RefOptional<C> CastTo(uint8_t* base) const
    {
        if (base == nullptr)
        {
            return std::nullopt;
        }

        return *(reinterpret_cast<C*>(base + Offset));
    }

	bool Include(const Range& other) const
	{
		if (other.Offset < Offset)
		{
            return false;
		}

		const auto offsetDiff = other.Offset - Offset;
        const auto maxValidSize = Size - offsetDiff;
        return maxValidSize >= other.Size;
	}

public:
    T Offset = {};
    T Size   = {};
};
}