/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <cstdint>
#include <functional>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace neolint
{
class RuleOptions
{
public:
    using StringList = std::vector<std::string>;
    using StringMap = std::map<std::string, std::string>;
    using Value = std::variant<bool, std::int64_t, std::string, StringList, StringMap>;
    enum class Type { Boolean, Integer, String, StringList, StringMap };

    bool Contains(std::string_view key) const;
    bool Empty() const noexcept;
    std::optional<Type> TypeOf(std::string_view key) const;
    std::vector<std::string> Keys() const;
    void Set(std::string key, Value value);
    bool GetBool(std::string_view key, bool fallback = false) const;
    std::int64_t GetInteger(std::string_view key, std::int64_t fallback = 0) const;
    std::string GetString(std::string_view key, std::string fallback = {}) const;
    StringList GetStrings(std::string_view key, StringList fallback = {}) const;
    StringMap GetStringMap(std::string_view key, StringMap fallback = {}) const;

private:
    std::map<std::string, Value, std::less<>> values_;
};
}