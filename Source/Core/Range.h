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

public:
    T Offset = {};
    T Size   = {};
};
}