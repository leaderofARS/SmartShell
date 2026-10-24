// SmartShell — Core Errors: Result<T> and Status
//
// Result<T> represents either a successfully computed value of type T
// or an Error explaining why computation failed.
//
// Status is Result<void> — represents success/failure without a value.
//
// Design goals:
//   - No hidden exceptions for control flow
//   - Caller must explicitly inspect result before using value
//   - SS_TRY macro allows clean early-return propagation
//   - Works naturally with move semantics
#pragma once

#include "error.hpp"
#include "../foundation/assert.hpp"
#include <variant>
#include <utility>
#include <functional>

namespace ss {

// ---------------------------------------------------------------------------
// Result<T>
// ---------------------------------------------------------------------------
template<typename T>
class Result {
public:
    // ---- Construction -------------------------------------------------------

    /// Construct a successful result
    static Result ok(T value) {
        Result r;
        r.m_storage.template emplace<0>(std::move(value));
        return r;
    }

    /// Construct a failed result
    static Result err(Error error) {
        Result r;
        r.m_storage.template emplace<1>(std::move(error));
        return r;
    }

    // Allow implicit construction from T (convenience)
    Result(T value) { m_storage.template emplace<0>(std::move(value)); }  // NOLINT

    // Allow implicit construction from Error (convenience)
    Result(Error error) { m_storage.template emplace<1>(std::move(error)); }  // NOLINT

    // ---- State query --------------------------------------------------------

    [[nodiscard]] bool isOk()    const noexcept { return m_storage.index() == 0; }
    [[nodiscard]] bool isError() const noexcept { return m_storage.index() == 1; }
    [[nodiscard]] explicit operator bool() const noexcept { return isOk(); }

    // ---- Value access -------------------------------------------------------

    [[nodiscard]] const T& value() const & {
        SS_ASSERT(isOk(), "Called value() on an error Result");
        return std::get<0>(m_storage);
    }

    [[nodiscard]] T& value() & {
        SS_ASSERT(isOk(), "Called value() on an error Result");
        return std::get<0>(m_storage);
    }

    [[nodiscard]] T&& value() && {
        SS_ASSERT(isOk(), "Called value() on an error Result");
        return std::get<0>(std::move(m_storage));
    }

    [[nodiscard]] const Error& error() const & {
        SS_ASSERT(isError(), "Called error() on an ok Result");
        return std::get<1>(m_storage);
    }

    [[nodiscard]] Error& error() & {
        SS_ASSERT(isError(), "Called error() on an ok Result");
        return std::get<1>(m_storage);
    }

    [[nodiscard]] Error&& error() && {
        SS_ASSERT(isError(), "Called error() on an ok Result");
        return std::get<1>(std::move(m_storage));
    }

    /// Return value if ok, otherwise return the provided default
    [[nodiscard]] T valueOr(T defaultValue) const & {
        if (isOk()) return std::get<0>(m_storage);
        return defaultValue;
    }

    [[nodiscard]] T valueOr(T defaultValue) && {
        if (isOk()) return std::get<0>(std::move(m_storage));
        return defaultValue;
    }

    /// Transform ok value, propagate error
    template<typename F>
    auto map(F&& fn) const & -> Result<std::invoke_result_t<F, const T&>> {
        if (isOk()) return fn(std::get<0>(m_storage));
        return std::get<1>(m_storage);
    }

private:
    Result() = default;
    std::variant<T, Error> m_storage;
};

// ---------------------------------------------------------------------------
// Status  (= Result<void>)
// ---------------------------------------------------------------------------
template<>
class Result<void> {
public:
    /// Successful status
    static Result ok() {
        Result r;
        r.m_ok = true;
        return r;
    }

    /// Failed status
    static Result err(Error error) {
        Result r;
        r.m_ok = false;
        r.m_error = std::move(error);
        return r;
    }

    // Allow implicit construction from Error
    Result(Error error) : m_ok{false}, m_error{std::move(error)} {}  // NOLINT

    [[nodiscard]] bool isOk()    const noexcept { return m_ok; }
    [[nodiscard]] bool isError() const noexcept { return !m_ok; }
    [[nodiscard]] explicit operator bool() const noexcept { return m_ok; }

    [[nodiscard]] const Error& error() const & {
        SS_ASSERT(!m_ok, "Called error() on an ok Status");
        return m_error;
    }

    [[nodiscard]] Error& error() & {
        SS_ASSERT(!m_ok, "Called error() on an ok Status");
        return m_error;
    }

private:
    Result() = default;
    bool  m_ok{false};
    Error m_error;
};

using Status = Result<void>;

// Convenience factory for ok status
inline Status statusOk() { return Status::ok(); }

// ---------------------------------------------------------------------------
// SS_TRY(expr) — propagate error from a Result/Status expression
//
// Usage (inside a function returning Result<X> or Status):
//   auto val = SS_TRY(someOperation());
//   // val is the unwrapped T if operation succeeded, otherwise function returns error
// ---------------------------------------------------------------------------
#define SS_TRY(expr) \
    ({ \
        auto _ss_result = (expr); \
        if (_ss_result.isError()) return std::move(_ss_result.error()); \
        std::move(_ss_result.value()); \
    })

// For void-returning operations (Status)
#define SS_TRY_STATUS(expr) \
    do { \
        auto _ss_result = (expr); \
        if (_ss_result.isError()) return std::move(_ss_result.error()); \
    } while (false)

} // namespace ss
