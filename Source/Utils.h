#pragma once
#include <Core.h>

namespace sy
{
    /************************ Strings ************************/
    template <size_t BufferSize = 512>
    std::wstring AnsiToWString(std::string_view ansiString)
    {
        std::array<wchar_t, BufferSize> buffer;
        MultiByteToWideChar(CP_ACP, 0, ansiString.data(), -1, buffer.data(), BufferSize);
        return std::wstring(buffer.data());
    }

    template <size_t BufferSize = 512>
    std::string WStringToAnsi(std::wstring_view wideString)
    {
        std::array<char, BufferSize> buffer;
        WideCharToMultiByte(CP_ACP, 0, wideString.data(), -1, buffer.data(), BufferSize, NULL, NULL);
        return std::string(buffer.data());
    }

    /************************ Memory ************************/
    inline size_t AlignForwardAdjustment(const size_t offset, size_t alignment) noexcept
    {
        const size_t adjustment = alignment - (offset & (alignment - 1));
        if (adjustment == alignment)
        {
            return 0;
        }

        return adjustment;
    }

    /************************ Helpers ************************/
    template<typename T>
    [[nodiscard]] bool FlagsContains(T flags, T flag) noexcept
    {
        return (flags & flag) != 0;
    }
}
