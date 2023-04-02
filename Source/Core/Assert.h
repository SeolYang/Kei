#pragma once
#include <PCH.h>

#if defined(_DEBUG) || defined(DEBUG)
#    define SY_ASSERT(CONDITION, FORMAT_STR, ...)                                            \
        do                                                                                   \
            if (!(CONDITION))                                                                \
            {                                                                                \
                spdlog::critical("Assert failed at File: {}, Line: {}", __FILE__, __LINE__); \
                spdlog::critical(FORMAT_STR, __VA_ARGS__);                                   \
                __debugbreak();                                                              \
            }                                                                                \
        while (0)
#else
#    define SY_ASSERT(CONDITION, FORMAT_STR, ...)                                                        \
        do                                                                                   \
            if (!(CONDITION))                                                                \
            {                                                                                \
                spdlog::critical("Assert failed at File: {}, Line: {}", __FILE__, __LINE__); \
                spdlog::critical(FORMAT_STR, __VA_ARGS__);                                   \
            }                                                                                \
        while (0)
#endif

#if defined(_DEBUG) || defined(DEUG)
#    define VK_ASSERT(RESULT, FORMAT_STR, ...) \
        SY_ASSERT((RESULT == VK_SUCCESS), FORMAT_STR, __VA_ARGS__)
#else
#    define VK_ASSERT(CONDITION, ...) \
        do                            \
        {                             \
            CONDITION;                \
        } while (0)
#endif