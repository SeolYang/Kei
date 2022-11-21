#pragma once
#include <Core.h>

#if defined(_DEBUG) || defined(DEBUG)
#define SY_ASSERT(CONDITION, FORMAT_STR, ...) \
    if (!(CONDITION)) { \
		spdlog::critical("Assert failed at File: {}, Line: {}", __FILE__, __LINE__); \
        spdlog::critical(FORMAT_STR, __VA_ARGS__); \
        __debugbreak(); \
        }
#elif
#define SY_ASSERT(...)
#endif

#if defined(_DEBUG) || defined(DEUG)
#define VK_ASSERT(RESULT, FORMAT_STR, ...) \
    SY_ASSERT((RESULT == VK_SUCCESS), FORMAT_STR, __VA_ARGS__)
#elif
#endif