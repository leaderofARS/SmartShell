// SmartShell -- Core Configuration: Hierarchical Node Store
//
// ConfigNode holds a tree of ConfigValues addressable by ConfigPath.
// It represents a single configuration layer (e.g., "user" or "defaults").
#pragma once

#include "value.hpp"
#include "path.hpp"
#include "../errors/result.hpp"
#include <optional>
#include <map>

namespace ss {

class ConfigNode {
public:
    ConfigNode() : m_value{ConfigValue{ConfigValue::ObjectMap{}}} {}

    // ---- Read ---------------------------------------------------------------

    /// Get the value at path, or nullopt if not found.
    [[nodiscard]] std::optional<ConfigValue> get(const ConfigPath& path) const {
        if (path.empty()) return m_value;
        if (!m_value.isObject()) return std::nullopt;

        const auto& head = path.head();
        const auto& obj  = m_value.asObject();
        auto it = obj.find(head);
        if (it == obj.end()) return std::nullopt;

        if (path.size() == 1) return it->second;

        // Recurse into child node
        ConfigNode child;
        child.m_value = it->second;
        return child.get(path.tail());
    }

    [[nodiscard]] bool contains(const ConfigPath& path) const {
        return get(path).has_value();
    }

    // ---- Write --------------------------------------------------------------

    /// Set a value at the given path, creating intermediate objects as needed.
    Status set(const ConfigPath& path, ConfigValue value) {
        if (path.empty()) {
            m_value = std::move(value);
            return Status::ok();
        }
        if (!m_value.isObject()) {
            m_value = ConfigValue{ConfigValue::ObjectMap{}};
        }
        return setInObject(m_value.asObject(), path, std::move(value));
    }

    /// Remove a key. Returns error if path doesn't exist.
    Status remove(const ConfigPath& path) {
        if (path.empty() || !m_value.isObject()) {
            return makeError(ErrorCategory::NotFound, "ConfigPath not found", "remove");
        }
        auto& obj = m_value.asObject();
        if (path.size() == 1) {
            auto it = obj.find(path.head());
            if (it == obj.end()) {
                return makeError(ErrorCategory::NotFound, "Key not found", "remove",
                                 path.toString());
            }
            obj.erase(it);
            return Status::ok();
        }
        auto it = obj.find(path.head());
        if (it == obj.end()) {
            return makeError(ErrorCategory::NotFound, "Key not found", "remove",
                             path.head());
        }
        ConfigNode child;
        child.m_value = it->second;
        auto st = child.remove(path.tail());
        if (st.isOk()) it->second = child.m_value;
        return st;
    }

    /// Return top-level key names (only valid when the root is an Object).
    [[nodiscard]] std::vector<Utf8String> keys() const {
        std::vector<Utf8String> result;
        if (!m_value.isObject()) return result;
        for (auto& [k, _] : m_value.asObject()) result.push_back(k);
        return result;
    }

    [[nodiscard]] const ConfigValue& rawValue() const noexcept { return m_value; }

private:
    ConfigValue m_value;

    static Status setInObject(ConfigValue::ObjectMap& obj,
                              const ConfigPath& path, ConfigValue value) {
        const auto& head = path.head();
        if (path.size() == 1) {
            obj[head] = std::move(value);
            return Status::ok();
        }
        // Descend or create intermediate object
        auto it = obj.find(head);
        if (it == obj.end() || !it->second.isObject()) {
            obj[head] = ConfigValue{ConfigValue::ObjectMap{}};
        }
        return setInObject(obj[head].asObject(), path.tail(), std::move(value));
    }
};

} // namespace ss
