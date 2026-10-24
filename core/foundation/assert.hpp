// SmartShell — Core Foundation: Assertions
// SS_ASSERT: programmer invariants that must never be violated.
// SS_UNREACHABLE: marks code paths that must never execute.
#pragma once

#include "compiler.hpp"
#include <cstdio>
#include <cstdlib>

namespace ss::detail {

SS_NORETURN inline void assertFail(
    const char* expr, const char* msg,
    const char* file, int line, const char* func) noexcept
{
    std::fprintf(stderr,
        "\n[SmartShell] Assertion failed: %s\n"
        "  Message : %s\n"
        "  Location: %s:%d in %s\n",
        expr, msg ? msg : "(none)", file, line, func);
    std::fflush(stderr);
#if defined(SS_MSVC)
    __debugbreak();
#elif defined(SS_GCC) || defined(SS_CLANG)
    __builtin_trap();
#else
    std::abort();
#endif
}

} // namespace ss::detail

// SS_ASSERT — active in both Debug and Release to catch programmer errors.
// In performance-critical release code, wrap with SS_DEBUG_BUILD if needed.
#define SS_ASSERT(expr, msg) \
    do { \
        if (SS_UNLIKELY(!(expr))) { \
            ::ss::detail::assertFail(#expr, msg, __FILE__, __LINE__, __func__); \
        } \
    } while (false)

// SS_ASSERT_DEBUG — only active in debug builds
#ifdef SS_DEBUG_BUILD
#  define SS_ASSERT_DEBUG(expr, msg) SS_ASSERT(expr, msg)
#else
#  define SS_ASSERT_DEBUG(expr, msg) do {} while (false)
#endif

// SS_UNREACHABLE — marks logically impossible code paths
#if defined(SS_GCC) || defined(SS_CLANG)
#  define SS_UNREACHABLE() \
     do { \
         ::ss::detail::assertFail("UNREACHABLE", "Reached unreachable code path", \
                                  __FILE__, __LINE__, __func__); \
         __builtin_unreachable(); \
     } while (false)
#elif defined(SS_MSVC)
#  define SS_UNREACHABLE() \
     do { \
         ::ss::detail::assertFail("UNREACHABLE", "Reached unreachable code path", \
                                  __FILE__, __LINE__, __func__); \
         __assume(false); \
     } while (false)
#else
#  define SS_UNREACHABLE() \
     ::ss::detail::assertFail("UNREACHABLE", "Reached unreachable code path", \
                              __FILE__, __LINE__, __func__)
#endif
