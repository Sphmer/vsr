// Simplified JSON header for VSR C++ port
// Using nlohmann/json concepts but simplified for our needs

#pragma once

#include <string>
#include <vector>
#include <map>
#include <any>
#include <variant>
#include <iostream>
#include <sstream>
#include <fstream>

namespace nlohmann {

// JSON value types
enum class value_t : std::uint8_t {
    null,
    object,
    array,
    string,
    boolean,
    number_integer,
    number_unsigned,
    number_float,
    binary,
    discarded
};

class json {
public:
    using object_t = std::map<std::string, json>;
    using array_t = std::vector<json>;
    using string_t = std::string;
    using boolean_t = bool;
    using number_integer_t = std::int64_t;
    using number_unsigned_t = std::uint64_t;
    using number_float_t = double;

private:
    value_t m_type = value_t::null;
    std::variant<
        std::nullptr_t,
        object_t,
        array_t,
        string_t,
        boolean_t,
        number_integer_t,
        number_unsigned_t,
        number_float_t
    > m_value;

public:
    // Constructors
    json() = default;
    json(std::nullptr_t) : m_type(value_t::null), m_value(nullptr) {}
    json(const object_t& obj) : m_type(value_t::object), m_value(obj) {}
    json(const array_t& arr) : m_type(value_t::array), m_value(arr) {}
    json(const string_t& str) : m_type(value_t::string), m_value(str) {}
    json(const char* str) : m_type(value_t::string), m_value(string_t(str)) {}
    json(boolean_t b) : m_type(value_t::boolean), m_value(b) {}
    json(number_integer_t i) : m_type(value_t::number_integer), m_value(i) {}
    json(number_unsigned_t u) : m_type(value_t::number_unsigned), m_value(u) {}
    json(number_float_t f) : m_type(value_t::number_float), m_value(f) {}
    json(int i) : json(static_cast<number_integer_t>(i)) {}
    json(double d) : json(static_cast<number_float_t>(d)) {}

    // Type checking
    bool is_null() const { return m_type == value_t::null; }
    bool is_object() const { return m_type == value_t::object; }
    bool is_array() const { return m_type == value_t::array; }
    bool is_string() const { return m_type == value_t::string; }
    bool is_boolean() const { return m_type == value_t::boolean; }
    bool is_number() const { 
        return m_type == value_t::number_integer || 
               m_type == value_t::number_unsigned || 
               m_type == value_t::number_float; 
    }
    bool is_number_integer() const { return m_type == value_t::number_integer; }
    bool is_number_unsigned() const { return m_type == value_t::number_unsigned; }
    bool is_number_float() const { return m_type == value_t::number_float; }

    // Value access
    object_t& get_ref<object_t&>() { return std::get<object_t>(m_value); }
    const object_t& get_ref<const object_t&>() const { return std::get<object_t>(m_value); }
    array_t& get_ref<array_t&>() { return std::get<array_t>(m_value); }
    const array_t& get_ref<const array_t&>() const { return std::get<array_t>(m_value); }
    
    template<typename T>
    T get() const {
        if constexpr (std::is_same_v<T, string_t>) {
            return std::get<string_t>(m_value);
        } else if constexpr (std::is_same_v<T, boolean_t>) {
            return std::get<boolean_t>(m_value);
        } else if constexpr (std::is_same_v<T, number_integer_t>) {
            return std::get<number_integer_t>(m_value);
        } else if constexpr (std::is_same_v<T, number_unsigned_t>) {
            return std::get<number_unsigned_t>(m_value);
        } else if constexpr (std::is_same_v<T, number_float_t>) {
            return std::get<number_float_t>(m_value);
        }
    }

    // Array/Object access
    json& operator[](const std::string& key) {
        if (m_type == value_t::null) {
            m_type = value_t::object;
            m_value = object_t{};
        }
        return std::get<object_t>(m_value)[key];
    }

    json& operator[](std::size_t idx) {
        if (m_type == value_t::null) {
            m_type = value_t::array;
            m_value = array_t{};
        }
        auto& arr = std::get<array_t>(m_value);
        if (idx >= arr.size()) {
            arr.resize(idx + 1);
        }
        return arr[idx];
    }

    const json& operator[](const std::string& key) const {
        return std::get<object_t>(m_value).at(key);
    }

    const json& operator[](std::size_t idx) const {
        return std::get<array_t>(m_value).at(idx);
    }

    // Utility methods
    bool contains(const std::string& key) const {
        if (m_type != value_t::object) return false;
        const auto& obj = std::get<object_t>(m_value);
        return obj.find(key) != obj.end();
    }

    std::size_t size() const {
        switch (m_type) {
            case value_t::object:
                return std::get<object_t>(m_value).size();
            case value_t::array:
                return std::get<array_t>(m_value).size();
            case value_t::string:
                return std::get<string_t>(m_value).size();
            default:
                return 0;
        }
    }

    bool empty() const {
        return size() == 0;
    }

    // Iterators for objects
    auto begin() {
        if (m_type == value_t::object) {
            return std::get<object_t>(m_value).begin();
        } else if (m_type == value_t::array) {
            return std::get<array_t>(m_value).begin();
        }
        throw std::runtime_error("Cannot iterate over non-container type");
    }

    auto end() {
        if (m_type == value_t::object) {
            return std::get<object_t>(m_value).end();
        } else if (m_type == value_t::array) {
            return std::get<array_t>(m_value).end();
        }
        throw std::runtime_error("Cannot iterate over non-container type");
    }

    auto begin() const {
        if (m_type == value_t::object) {
            return std::get<object_t>(m_value).begin();
        } else if (m_type == value_t::array) {
            return std::get<array_t>(m_value).begin();
        }
        throw std::runtime_error("Cannot iterate over non-container type");
    }

    auto end() const {
        if (m_type == value_t::object) {
            return std::get<object_t>(m_value).end();
        } else if (m_type == value_t::array) {
            return std::get<array_t>(m_value).end();
        }
        throw std::runtime_error("Cannot iterate over non-container type");
    }

    // Parsing
    static json parse(const std::string& str);
    static json parse(std::istream& stream);

    // Serialization
    std::string dump(int indent = -1) const;
    friend std::ostream& operator<<(std::ostream& os, const json& j);
};

} // namespace nlohmann

// Alias for convenience
using json = nlohmann::json;
