// SmartShell -- Core Configuration: Value Types
//
// ConfigValue is a typed variant that can represent any configuration datum:
// null, bool, integer, float, string, array, or nested object (map).
#pragma once

#include "../foundation/types.hpp"
#include "../foundation/assert.hpp"
#include "../text/encoding.hpp"
#include <variant>
#include <vector>
#include <map>
#include <string>

namespace ss {

enum class ConfigValueType {
    Null,
    Bool,
    Integer,
    Float,
    String,
    Array,
    Object,
};

class ConfigValue {
public:
    using ObjectMap = std::map<Utf8String, ConfigValue>;
    using ArrayVec  = std::vector<ConfigValue>;

    // -- Construction ---------------------------------------------------------
    ConfigValue()                          : m_data{std::monostate{}}      {} // Null
    explicit ConfigValue(bool v)           : m_data{v}                      {}
    explicit ConfigValue(i64 v)            : m_data{v}                      {}
    explicit ConfigValue(double v)         : m_data{v}                      {}
    explicit ConfigValue(Utf8String v)     : m_data{std::move(v)}           {}
    explicit ConfigValue(Utf8View v)       : m_data{Utf8String{v}}          {}
    explicit ConfigValue(ArrayVec v)       : m_data{std::move(v)}           {}
    explicit ConfigValue(ObjectMap v)      : m_data{std::move(v)}           {}

    // Convenience integer overloads
    explicit ConfigValue(int v)            : m_data{static_cast<i64>(v)}    {}
    explicit ConfigValue(u32 v)            : m_data{static_cast<i64>(v)}    {}

    // -- Type query -----------------------------------------------------------
    [[nodiscard]] ConfigValueType type() const noexcept {
        return static_cast<ConfigValueType>(m_data.index());
    }

    [[nodiscard]] bool isNull()    const noexcept { return m_data.index() == 0; }
    [[nodiscard]] bool isBool()    const noexcept { return m_data.index() == 1; }
    [[nodiscard]] bool isInteger() const noexcept { return m_data.index() == 2; }
    [[nodiscard]] bool isFloat()   const noexcept { return m_data.index() == 3; }
    [[nodiscard]] bool isString()  const noexcept { return m_data.index() == 4; }
    [[nodiscard]] bool isArray()   const noexcept { return m_data.index() == 5; }
    [[nodiscard]] bool isObject()  const noexcept { return m_data.index() == 6; }

    // -- Typed access ---------------------------------------------------------
    [[nodiscard]] bool asBool() const {
        SS_ASSERT(isBool(), "ConfigValue::asBool() called on non-bool");
        return std::get<bool>(m_data);
    }
    [[nodiscard]] i64 asInteger() const {
        SS_ASSERT(isInteger(), "ConfigValue::asInteger() called on non-integer");
        return std::get<i64>(m_data);
    }
    [[nodiscard]] double asFloat() const {
        SS_ASSERT(isFloat(), "ConfigValue::asFloat() called on non-float");
        return std::get<double>(m_data);
    }
    [[nodiscard]] const Utf8String& asString() const {
        SS_ASSERT(isString(), "ConfigValue::asString() called on non-string");
        return std::get<Utf8String>(m_data);
    }
    [[nodiscard]] const ArrayVec& asArray() const {
        SS_ASSERT(isArray(), "ConfigValue::asArray() called on non-array");
        return std::get<ArrayVec>(m_data);
    }
    [[nodiscard]] const ObjectMap& asObject() const {
        SS_ASSERT(isObject(), "ConfigValue::asObject() called on non-object");
        return std::get<ObjectMap>(m_data);
    }

    // Mutable references
    [[nodiscard]] ArrayVec& asArray() {
        SS_ASSERT(isArray(), "ConfigValue::asArray() on non-array");
        return std::get<ArrayVec>(m_data);
    }
    [[nodiscard]] ObjectMap& asObject() {
        SS_ASSERT(isObject(), "ConfigValue::asObject() on non-object");
        return std::get<ObjectMap>(m_data);
    }

    bool operator==(const ConfigValue& o) const noexcept { return m_data == o.m_data; }

private:
    std::variant<
        std::monostate, // Null  (index 0)
        bool,           // Bool  (index 1)
        i64,            // Integer (index 2)
        double,         // Float (index 3)
        Utf8String,     // String (index 4)
        ArrayVec,       // Array  (index 5)
        ObjectMap       // Object (index 6)
    > m_data;
};

} // namespace ss
