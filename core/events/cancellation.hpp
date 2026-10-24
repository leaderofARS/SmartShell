// SmartShell -- Core Events: Cancellation Token/Source
//
// CancellationSource -- owns and controls the cancellation state.
//                       Call cancel() to request cancellation.
// CancellationToken  -- non-owning observer. Check isCancelled() frequently
//                       in long-running operations.
//
// Both are cheap to copy. The underlying state is reference-counted.
// Thread-safety: cancel() and isCancelled() are safe from any thread.
#pragma once

#include <atomic>
#include <memory>

namespace ss {

namespace detail {
struct CancellationState {
    std::atomic<bool> cancelled{false};
};
} // namespace detail

// ---------------------------------------------------------------------------
// CancellationToken -- observer (non-owning view of cancellation state)
// ---------------------------------------------------------------------------
class CancellationToken {
public:
    CancellationToken() = default;
    explicit CancellationToken(std::shared_ptr<detail::CancellationState> state)
        : m_state{std::move(state)} {}

    [[nodiscard]] bool isCancelled() const noexcept {
        return m_state && m_state->cancelled.load(std::memory_order_relaxed);
    }

    [[nodiscard]] bool isValid() const noexcept { return m_state != nullptr; }

    /// Block-free poll alias
    [[nodiscard]] explicit operator bool() const noexcept { return isCancelled(); }

private:
    std::shared_ptr<detail::CancellationState> m_state;
};

// ---------------------------------------------------------------------------
// CancellationSource -- owns the cancellation state
// ---------------------------------------------------------------------------
class CancellationSource {
public:
    CancellationSource()
        : m_state{std::make_shared<detail::CancellationState>()} {}

    /// Request cancellation. All tokens from token() will see isCancelled()==true.
    void cancel() noexcept {
        m_state->cancelled.store(true, std::memory_order_relaxed);
    }

    [[nodiscard]] bool isCancelled() const noexcept {
        return m_state->cancelled.load(std::memory_order_relaxed);
    }

    /// Get a token that reflects this source's cancellation state.
    [[nodiscard]] CancellationToken token() const {
        return CancellationToken{m_state};
    }

private:
    std::shared_ptr<detail::CancellationState> m_state;
};

} // namespace ss
