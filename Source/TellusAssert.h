#pragma once
#include <Core.h>

namespace sy
{
    static void Assert(bool bCondition, std::string_view file, size_t line, std::string_view message)
    {
#if defined(_DEBUG) || defined(DEBUG)
        if (!bCondition)
        {
            // @TODO: Logging here
            //std::format("Assert failed at File : {}, Line : {}, Message : {}", file, line, message)
            __debugbreak();
        }
#endif
    }
}

#define SY_ASSERT(Condition, LogMessage) sy::Assert(Condition, __FILE__, __LINE__, LogMessage);