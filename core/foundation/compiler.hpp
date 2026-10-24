// SmartShell — Core Foundation: Compiler Detection & Portability Macros
#pragma once

// ---------------------------------------------------------------------------
// Compiler detection
// ---------------------------------------------------------------------------
#if defined(_MSC_VER) && !defined(__clang__)
#  define SS_MSVC 1
#elif defined(__clang__)
#  define SS_CLANG 1
#elif defined(__GNUC__)
#  define SS_GCC 1
#endif

// ---------------------------------------------------------------------------
// Force-inline
// ---------------------------------------------------------------------------
#if defined(SS_MSVC)
#  define SS_FORCEINLINE __forceinline
#elif defined(SS_GCC) || defined(SS_CLANG)
#  define SS_FORCEINLINE __attribute__((always_inline)) inline
#else
#  define SS_FORCEINLINE inline
#endif

// ---------------------------------------------------------------------------
// No-return
// ---------------------------------------------------------------------------
#define SS_NORETURN [[noreturn]]

// ---------------------------------------------------------------------------
// Nodiscard helper
// ---------------------------------------------------------------------------
#define SS_NODISCARD [[nodiscard]]

// ---------------------------------------------------------------------------
// Branch-prediction hints
// ---------------------------------------------------------------------------
#if defined(SS_GCC) || defined(SS_CLANG)
#  define SS_LIKELY(x)   __builtin_expect(!!(x), 1)
#  define SS_UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
#  define SS_LIKELY(x)   (x)
#  define SS_UNLIKELY(x) (x)
#endif

// ---------------------------------------------------------------------------
// Debug build detection
// ---------------------------------------------------------------------------
#if !defined(NDEBUG)
#  define SS_DEBUG_BUILD 1
#endif
