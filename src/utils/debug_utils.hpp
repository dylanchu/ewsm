#pragma once

// includes
#ifdef _DEBUG
#include <algorithm>
#include <cstdio>    // IWYU pragma: keep
#include <windows.h> // IWYU pragma: keep
#include <iostream>
#include <tchar.h>

#else
#endif

namespace utils
{
    inline void SetupConsole(LPCWSTR title)
    {
#ifdef _DEBUG
        freopen("CONIN$", "r", stdin);
        freopen("CONOUT$", "w", stdout);
        freopen("CONOUT$", "w", stderr);
        ::SetConsoleTitle(title);
        printf("hello console\n");
#endif
    }
} // namespace utils

// DEBUG_MSG
#ifdef _DEBUG
#define DEBUG_MSG(fmt, ...)                                                       \
    do {                                                                          \
        std::printf("[DEBUG] %s:%d: " fmt "\n", __FILE__, __LINE__, __VA_ARGS__); \
    } while (0)
#else
#define DEBUG_MSG(fmt, ...) \
    do {                    \
        (void)0;            \
    } while (0)
#endif
