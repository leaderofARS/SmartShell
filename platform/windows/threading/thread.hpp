// SmartShell -- Platform Windows: Thread RAII Wrapper
#pragma once

#include "../../../core/errors/result.hpp"
#include "../../../core/identifiers/types.hpp"
#include "../handles/handle.hpp"
#include <functional>
#include <string>

namespace ss::platform::windows {

/// RAII wrapper around a Windows thread (CreateThread).
/// Non-copyable; movable.
/// Thread-safety: Only call join()/detach() from the owning thread.
class Thread {
public:
    Thread() = default;
    ~Thread();

    Thread(const Thread&)            = delete;
    Thread& operator=(const Thread&) = delete;
    Thread(Thread&&) noexcept;
    Thread& operator=(Thread&&) noexcept;

    /// Start a thread executing the given function.
    [[nodiscard]] Status start(std::function<void()> fn);

    /// Wait for the thread to finish. Safe to call only once.
    [[nodiscard]] Status join();

    /// Abandon the thread — it will run until completion on its own.
    void detach();

    /// Set a human-readable name visible in debuggers (uses SetThreadDescription).
    [[nodiscard]] Status setName(Utf8View name);

    [[nodiscard]] ThreadId id()         const noexcept { return m_id; }
    [[nodiscard]] bool     isJoinable() const noexcept { return m_handle.valid(); }

private:
    NullHandle m_handle;
    ThreadId   m_id;

    // Trampoline: owns the std::function to bridge Win32 LPTHREAD_START_ROUTINE
    struct ThreadData {
        std::function<void()> fn;
    };
    static DWORD WINAPI threadProc(LPVOID param);
};

// ---------------------------------------------------------------------------
// Mutex -- SRWLOCK-based (slim reader/writer lock)
// Significantly lighter than a CRITICAL_SECTION for typical use.
// ---------------------------------------------------------------------------
class Mutex {
public:
    Mutex()  noexcept { ::InitializeSRWLock(&m_lock); }
    ~Mutex() noexcept = default;

    Mutex(const Mutex&)            = delete;
    Mutex& operator=(const Mutex&) = delete;

    void lock()   noexcept { ::AcquireSRWLockExclusive(&m_lock); }
    void unlock() noexcept { ::ReleaseSRWLockExclusive(&m_lock); }

    // shared (read) access
    void lockShared()   noexcept { ::AcquireSRWLockShared(&m_lock); }
    void unlockShared() noexcept { ::ReleaseSRWLockShared(&m_lock); }

    [[nodiscard]] SRWLOCK* native() noexcept { return &m_lock; }

private:
    SRWLOCK m_lock{};
};

/// Exclusive (write) scoped lock
class ScopedLock {
public:
    explicit ScopedLock(Mutex& m) noexcept : m_mutex{m} { m_mutex.lock(); }
    ~ScopedLock() noexcept { m_mutex.unlock(); }
    ScopedLock(const ScopedLock&) = delete;
private:
    Mutex& m_mutex;
};

/// Shared (read) scoped lock
class SharedLock {
public:
    explicit SharedLock(Mutex& m) noexcept : m_mutex{m} { m_mutex.lockShared(); }
    ~SharedLock() noexcept { m_mutex.unlockShared(); }
    SharedLock(const SharedLock&) = delete;
private:
    Mutex& m_mutex;
};

} // namespace ss::platform::windows
