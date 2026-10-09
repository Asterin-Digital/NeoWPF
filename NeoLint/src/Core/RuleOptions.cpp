#include <NeoLint/Core/RuleOptions.h>

#include <utility>

namespace neolint
{
bool RuleOptions::Contains(std::string_view key) const
{
    return values_.contains(key);
}

bool RuleOptions::Empty() const noexcept
{
    return values_.empty();
}

std::vector<std::string> RuleOptions::Keys() const
{
    std::vector<std::string> keys;
    keys.reserve(values_.size());
    for (const auto& entry : values_) keys.push_back(entry.first);
    return keys;
}

void RuleOptions::Set(std::string key, Value value)
{
    values_.insert_or_assign(std::move(key), std::move(value));
}

bool RuleOptions::GetBool(std::string_view key, bool fallback) const
{
    const auto value = values_.find(key);
    if (value == values_.end()) return fallback;
    const auto* result = std::get_if<bool>(&value->second);
    return result ? *result : fallback;
}

std::int64_t RuleOptions::GetInteger(std::string_view key, std::int64_t fallback) const
{
    const auto value = values_.find(key);
    if (value == values_.end()) return fallback;
    const auto* result = std::get_if<std::int64_t>(&value->second);
    return result ? *result : fallback;
}

std::string RuleOptions::GetString(std::string_view key, std::string fallback) const
{
    const auto value = values_.find(key);
    if (value == values_.end()) return fallback;
    const auto* result = std::get_if<std::string>(&value->second);
    return result ? *result : fallback;
}

RuleOptions::StringList RuleOptions::GetStrings(
    std::string_view key, StringList fallback) const
{
    const auto value = values_.find(key);
    if (value == values_.end()) return fallback;
    const auto* result = std::get_if<StringList>(&value->second);
    return result ? *result : fallback;
}

RuleOptions::StringMap RuleOptions::GetStringMap(
    std::string_view key, StringMap fallback) const
{
    const auto value = values_.find(key);
    if (value == values_.end()) return fallback;
    const auto* result = std::get_if<StringMap>(&value->second);
    return result ? *result : fallback;
}

std::optional<RuleOptions::Type> RuleOptions::TypeOf(std::string_view key) const
{
    const auto value = values_.find(key);
    if (value == values_.end()) return std::nullopt;
    switch (value->second.index())
    {
    case 0: return Type::Boolean;
    case 1: return Type::Integer;
    case 2: return Type::String;
    case 3: return Type::StringList;
    case 4: return Type::StringMap;
    default: return std::nullopt;
    }
}
}