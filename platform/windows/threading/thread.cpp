// SmartShell -- Platform Windows: Thread Implementation
#include "thread.hpp"
#include "../../../core/text/encoding.hpp"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

namespace ss::platform::windows {

// ---- Thread::threadProc (static trampoline) ---------------------------------

DWORD WINAPI Thread::threadProc(LPVOID param) {
    // Take ownership of the ThreadData allocated in start()
    ThreadData* data = static_cast<ThreadData*>(param);
    try {
        data->fn();
    } catch (...) {
        // Threads must not propagate exceptions past their entry point
    }
    delete data;
    return 0;
}

// ---- Thread::~Thread --------------------------------------------------------

Thread::~Thread() {
    // If still joinable and not detached, we must join to avoid resource leak.
    // The caller should always join explicitly. We join here as a safety net.
    if (isJoinable()) {
        (void)join();
    }
}

Thread::Thread(Thread&& other) noexcept
    : m_handle{std::move(other.m_handle)}
    , m_id{other.m_id}
{
    other.m_id = ThreadId{};
}

Thread& Thread::operator=(Thread&& other) noexcept {
    if (this != &other) {
        if (isJoinable()) (void)join();
        m_handle = std::move(other.m_handle);
        m_id     = other.m_id;
        other.m_id = ThreadId{};
    }
    return *this;
}

// ---- Thread::start ----------------------------------------------------------

Status Thread::start(std::function<void()> fn) {
    if (isJoinable()) {
        return makeError(ErrorCategory::InvalidState,
                         "Thread already started", "Thread::start");
    }

    auto* data = new (std::nothrow) ThreadData{std::move(fn)};
    if (!data) {
        return makeError(ErrorCategory::ResourceExhausted,
                         "Failed to allocate ThreadData", "Thread::start");
    }

    DWORD tid = 0;
    HANDLE h = ::CreateThread(nullptr, 0, threadProc, data, 0, &tid);
    if (!h) {
        delete data;
        return makeLastError("CreateThread failed", "Thread::start");
    }

    m_handle = NullHandle{h};
    m_id     = ThreadId{static_cast<u32>(tid)};
    return Status::ok();
}

// ---- Thread::join -----------------------------------------------------------

Status Thread::join() {
    if (!isJoinable()) {
        return makeError(ErrorCategory::InvalidState,
                         "Thread not joinable", "Thread::join");
    }
    DWORD result = ::WaitForSingleObject(m_handle.get(), INFINITE);
    m_handle.reset();  // release the handle
    if (result != WAIT_OBJECT_0) {
        return makeLastError("WaitForSingleObject failed in Thread::join", "Thread::join");
    }
    return Status::ok();
}

// ---- Thread::detach ---------------------------------------------------------

void Thread::detach() {
    m_handle.reset();  // Release handle without joining; OS will clean up
}

// ---- Thread::setName --------------------------------------------------------

Status Thread::setName(Utf8View name) {
    if (!isJoinable()) {
        return makeError(ErrorCategory::InvalidState,
                         "Thread not started", "Thread::setName");
    }
    auto wide = toUtf16(name);
    if (wide.isError()) return std::move(wide.error());

    // SetThreadDescription is available on Windows 10 1607+
    using SetThreadDescFn = HRESULT(WINAPI*)(HANDLE, PCWSTR);
#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-function-type"
#endif
    static auto fn = reinterpret_cast<SetThreadDescFn>(
        ::GetProcAddress(::GetModuleHandleW(L"kernel32.dll"), "SetThreadDescription"));
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

    if (fn) {
        HRESULT hr = fn(m_handle.get(), wide.value().c_str());
        if (FAILED(hr)) {
            return makeError(ErrorCategory::PlatformFailure,
                             "SetThreadDescription failed", "Thread::setName");
        }
    }
    // If API not available, silently succeed (not a critical failure)
    return Status::ok();
}

} // namespace ss::platform::windows
