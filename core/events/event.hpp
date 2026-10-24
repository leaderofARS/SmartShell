// SmartShell -- Core Events: Event Types and Dispatcher
#pragma once

#include "../time/clock.hpp"
#include "../foundation/types.hpp"
#include <functional>
#include <unordered_map>
#include <vector>
#include <mutex>

namespace ss {

// ---------------------------------------------------------------------------
// EventType
// ---------------------------------------------------------------------------
enum class EventType : u32 {
    ApplicationStarted    = 0,
    ApplicationStopping   = 1,
    ConfigurationLoaded   = 2,
    ConfigurationChanged  = 3,
    ProcessStarted        = 4,
    ProcessExited         = 5,
    ResourceFailure       = 6,
};

// ---------------------------------------------------------------------------
// Event base
// ---------------------------------------------------------------------------
struct Event {
    EventType type;
    Timestamp timestamp{WallClock::now()};

    explicit Event(EventType t) : type{t} {}
    virtual ~Event() = default;
};

struct ApplicationStartedEvent : Event {
    ApplicationStartedEvent() : Event{EventType::ApplicationStarted} {}
};

struct ApplicationStoppingEvent : Event {
    ApplicationStoppingEvent() : Event{EventType::ApplicationStopping} {}
};

struct ConfigurationLoadedEvent : Event {
    ConfigurationLoadedEvent() : Event{EventType::ConfigurationLoaded} {}
};

struct ConfigurationChangedEvent : Event {
    Utf8String key;
    ConfigurationChangedEvent() : Event{EventType::ConfigurationChanged} {}
};

struct ProcessStartedEvent : Event {
    u32 nativePid{0};
    ProcessStartedEvent() : Event{EventType::ProcessStarted} {}
};

struct ProcessExitedEvent : Event {
    u32 nativePid{0};
    int exitCode{0};
    ProcessExitedEvent() : Event{EventType::ProcessExited} {}
};

struct ResourceFailureEvent : Event {
    Utf8String resource;
    Utf8String reason;
    ResourceFailureEvent() : Event{EventType::ResourceFailure} {}
};

// ---------------------------------------------------------------------------
// EventDispatcher -- synchronous delivery
// Thread-safety: subscribe/dispatch are protected by a mutex.
// ---------------------------------------------------------------------------
class EventDispatcher {
public:
    using Handler      = std::function<void(const Event&)>;
    using SubscriberId = u32;

    EventDispatcher() = default;

    /// Register a handler for a specific event type.
    [[nodiscard]] SubscriberId subscribe(EventType type, Handler handler) {
        std::lock_guard<std::mutex> lk{m_mutex};
        SubscriberId id = m_nextId++;
        m_handlers[static_cast<u32>(type)].push_back({id, std::move(handler)});
        return id;
    }

    /// Synchronously deliver an event to all registered handlers.
    void dispatch(const Event& event) noexcept {
        std::vector<Handler> toCall;
        {
            std::lock_guard<std::mutex> lk{m_mutex};
            auto it = m_handlers.find(static_cast<u32>(event.type));
            if (it == m_handlers.end()) return;
            toCall.reserve(it->second.size());
            for (auto& [id, fn] : it->second) toCall.push_back(fn);
        }
        for (auto& fn : toCall) {
            try { fn(event); } catch (...) {}
        }
    }

private:
    struct HandlerEntry { SubscriberId id; Handler fn; };
    std::mutex m_mutex;
    u32        m_nextId{1};
    std::unordered_map<u32, std::vector<HandlerEntry>> m_handlers;
};

} // namespace ss
