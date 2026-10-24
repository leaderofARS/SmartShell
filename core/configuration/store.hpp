// SmartShell -- Core Configuration: Layered Config Store
//
// Provides precedence-ordered key lookup across multiple named layers:
//   defaults < machine < user < profile < session < runtime
//
// Consumers call getEffective(path) to receive the highest-priority value.
// Each layer is an independent ConfigNode.
#pragma once

#include "node.hpp"
#include <array>
#include <string_view>

namespace ss {

enum class ConfigLayer : int {
    Defaults  = 0,
    Machine   = 1,
    User      = 2,
    Profile   = 3,
    Session   = 4,
    Runtime   = 5,
    kCount_   = 6,
};

class ConfigStore {
public:
    ConfigStore() = default;

    // ---- Write to a specific layer -----------------------------------------

    Status set(ConfigLayer layer, const ConfigPath& path, ConfigValue value) {
        return layer_at(layer).set(path, std::move(value));
    }

    Status set(ConfigLayer layer, Utf8View dotPath, ConfigValue value) {
        return set(layer, ConfigPath::parse(dotPath), std::move(value));
    }

    // ---- Read effective value (highest-priority layer wins) ----------------

    [[nodiscard]] std::optional<ConfigValue> getEffective(const ConfigPath& path) const {
        // Iterate from highest priority to lowest
        for (int i = static_cast<int>(ConfigLayer::kCount_) - 1; i >= 0; --i) {
            auto v = m_layers[static_cast<usize>(i)].get(path);
            if (v.has_value()) return v;
        }
        return std::nullopt;
    }

    [[nodiscard]] std::optional<ConfigValue> getEffective(Utf8View dotPath) const {
        return getEffective(ConfigPath::parse(dotPath));
    }

    // ---- Convenience typed accessors ---------------------------------------

    [[nodiscard]] std::optional<bool>       getBool(Utf8View p) const {
        auto v = getEffective(p);
        if (v && v->isBool()) return v->asBool();
        return std::nullopt;
    }
    [[nodiscard]] std::optional<i64>        getInteger(Utf8View p) const {
        auto v = getEffective(p);
        if (v && v->isInteger()) return v->asInteger();
        return std::nullopt;
    }
    [[nodiscard]] std::optional<Utf8String> getString(Utf8View p) const {
        auto v = getEffective(p);
        if (v && v->isString()) return v->asString();
        return std::nullopt;
    }

    // ---- Direct layer access -----------------------------------------------

    [[nodiscard]] const ConfigNode& layer(ConfigLayer l) const { return layer_at(l); }
    [[nodiscard]]       ConfigNode& layer(ConfigLayer l)       { return layer_at(l); }

private:
    std::array<ConfigNode, static_cast<usize>(ConfigLayer::kCount_)> m_layers;

    ConfigNode& layer_at(ConfigLayer l) {
        return m_layers[static_cast<usize>(l)];
    }
    const ConfigNode& layer_at(ConfigLayer l) const {
        return m_layers[static_cast<usize>(l)];
    }
};

} // namespace ss
