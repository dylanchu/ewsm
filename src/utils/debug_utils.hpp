#pragma once

// includes
#ifdef _DEBUG
    #include <algorithm>
    #include <cstdio> // IWYU pragma: keep
    #include <iostream>
    #include <tchar.h>
    #include <windows.h> // IWYU pragma: keep
    #include <filesystem> // IWYU pragma: keep
    #include <format> // IWYU pragma: keep
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
    #ifndef __FILENAME__
        #define __FILENAME__ std::filesystem::path(__FILE__).filename().string()
    #endif
    template <typename... Args>
    void _debug_msg_impl(std::string_view file, int line, const std::format_string<Args...>& fmt, Args&&... args)
    {
        if constexpr (sizeof...(Args) == 0) {
            std::cout << std::format("[DEBUG] [{}:{}] {}\n", file, line, fmt.get());
        }
        else {
            std::cout << std::format("[DEBUG] [{}:{}] {}\n", file, line, std::format(fmt, std::forward<Args>(args)...));
        }
    }
    // #define DEBUG_MSG(fmt, ...) _debug_msg_impl(__FILE__, __LINE__, fmt __VA_OPT__(,) __VA_ARGS__)
    #define DEBUG_MSG(fmt, ...) _debug_msg_impl(__FILENAME__, __LINE__, fmt, ##__VA_ARGS__)
#else
    #define DEBUG_MSG(fmt, ...) \
        do {                    \
            (void)0;            \
        } while (0)
#endif
