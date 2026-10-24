// SmartShell -- Platform Windows: RAII Handle Wrapper
//
// UniqueHandle<Traits> owns a single Windows HANDLE and closes it on destruction.
// It is non-copyable and movable.
//
// Traits must provide:
//   static HANDLE invalidValue() -- the sentinel "not a handle" value
//   static void   close(HANDLE)  -- how to close the handle
//
// Two built-in trait sets are provided:
//   KernelHandleTraits  -- invalid = INVALID_HANDLE_VALUE, close = CloseHandle
//   NullHandleTraits    -- invalid = nullptr,              close = CloseHandle
#pragma once

#include "../../../core/foundation/assert.hpp"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <utility>

namespace ss::platform::windows {

// ---------------------------------------------------------------------------
// Built-in trait sets
// ---------------------------------------------------------------------------

/// For handles returned by CreateFile, CreatePipe, etc. (invalid = INVALID_HANDLE_VALUE)
struct KernelHandleTraits {
    static HANDLE invalidValue() noexcept { return INVALID_HANDLE_VALUE; }
    static void   close(HANDLE h) noexcept { ::CloseHandle(h); }
};

/// For handles returned by CreateEvent, CreateMutex, OpenProcess, etc. (invalid = nullptr)
struct NullHandleTraits {
    static HANDLE invalidValue() noexcept { return nullptr; }
    static void   close(HANDLE h) noexcept { ::CloseHandle(h); }
};

// ---------------------------------------------------------------------------
// UniqueHandle<Traits>
// ---------------------------------------------------------------------------
template<typename Traits>
class UniqueHandle {
public:
    using HandleType = HANDLE;

    /// Construct an invalid (empty) handle.
    UniqueHandle() noexcept : m_handle{Traits::invalidValue()} {}

    /// Acquire ownership of an existing handle.
    explicit UniqueHandle(HandleType h) noexcept : m_handle{h} {}

    // Non-copyable
    UniqueHandle(const UniqueHandle&)            = delete;
    UniqueHandle& operator=(const UniqueHandle&) = delete;

    // Movable
    UniqueHandle(UniqueHandle&& other) noexcept
        : m_handle{other.m_handle} {
        other.m_handle = Traits::invalidValue();
    }

    UniqueHandle& operator=(UniqueHandle&& other) noexcept {
        if (this != &other) {
            reset();
            m_handle = other.m_handle;
            other.m_handle = Traits::invalidValue();
        }
        return *this;
    }

    ~UniqueHandle() noexcept { reset(); }

    // ---- State query -------------------------------------------------------

    [[nodiscard]] bool valid() const noexcept {
        return m_handle != Traits::invalidValue();
    }
    [[nodiscard]] explicit operator bool() const noexcept { return valid(); }

    // ---- Access ------------------------------------------------------------

    /// Borrow the raw handle (caller must not close it).
    [[nodiscard]] HandleType get() const noexcept { return m_handle; }

    // ---- Ownership transfer ------------------------------------------------

    /// Release ownership without closing. Caller becomes responsible.
    [[nodiscard]] HandleType release() noexcept {
        HandleType h = m_handle;
        m_handle = Traits::invalidValue();
        return h;
    }

    /// Close the current handle and optionally take a new one.
    void reset(HandleType h = Traits::invalidValue()) noexcept {
        if (m_handle != Traits::invalidValue()) {
            Traits::close(m_handle);
        }
        m_handle = h;
    }

private:
    HandleType m_handle;
};

// ---------------------------------------------------------------------------
// Convenience type aliases
// ---------------------------------------------------------------------------
using KernelHandle = UniqueHandle<KernelHandleTraits>;  ///< Files, pipes, etc.
using NullHandle   = UniqueHandle<NullHandleTraits>;    ///< Events, mutexes, processes

} // namespace ss::platform::windows
