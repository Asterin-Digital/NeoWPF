#include <NeoLint/Core/RuleConfig.h>

#include <nlohmann/json.hpp>

#include <fstream>
#include <cstdint>
#include <string>
#include <utility>

namespace neolint
{
namespace
{
using Json = nlohmann::ordered_json;

bool ParseSeverity(const std::string& value, RuleSeverity& severity)
{
    if (value == "error") severity = RuleSeverity::Error;
    else if (value == "warning") severity = RuleSeverity::Warning;
    else if (value == "info") severity = RuleSeverity::Info;
    else return false;
    return true;
}

bool MatchesPattern(std::string_view value, std::string_view pattern)
{
    std::size_t valueIndex = 0;
    std::size_t patternIndex = 0;
    std::size_t starIndex = std::string_view::npos;
    std::size_t matchIndex = 0;

    while (valueIndex < value.size())
    {
        if (patternIndex < pattern.size() &&
            (pattern[patternIndex] == '?' || pattern[patternIndex] == value[valueIndex]))
        {
            ++valueIndex;
            ++patternIndex;
        }
        else if (patternIndex < pattern.size() && pattern[patternIndex] == '*')
        {
            starIndex = patternIndex++;
            matchIndex = valueIndex;
        }
        else if (starIndex != std::string_view::npos)
        {
            patternIndex = starIndex + 1;
            valueIndex = ++matchIndex;
        }
        else
        {
            return false;
        }
    }

    while (patternIndex < pattern.size() && pattern[patternIndex] == '*') ++patternIndex;
    return patternIndex == pattern.size();
}

bool ParseOptions(const Json& value, RuleOptions& options,
                  const std::string& ruleId, std::string& error)
{
    if (!value.is_object())
    {
        error = "rule '" + ruleId + "' setting 'options' must be an object";
        return false;
    }
    for (const auto& [key, option] : value.items())
    {
        if (option.is_boolean()) options.Set(key, option.get<bool>());
        else if (option.is_number_integer()) options.Set(key, option.get<std::int64_t>());
        else if (option.is_string()) options.Set(key, option.get<std::string>());
        else if (option.is_array())
        {
            RuleOptions::StringList values;
            for (const auto& item : option)
            {
                if (!item.is_string())
                {
                    error = "rule '" + ruleId + "' option '" + key +
                        "' must contain only strings";
                    return false;
                }
                values.push_back(item.get<std::string>());
            }
            options.Set(key, std::move(values));
        }
        else if (option.is_object())
        {
            RuleOptions::StringMap values;
            for (const auto& [name, mapped] : option.items())
            {
                if (!mapped.is_string())
                {
                    error = "rule '" + ruleId + "' option '" + key +
                        "' must map strings to strings";
                    return false;
                }
                values.emplace(name, mapped.get<std::string>());
            }
            options.Set(key, std::move(values));
        }
        else
        {
            error = "rule '" + ruleId + "' option '" + key +
                "' must be a boolean, integer, string, string array, or string map";
            return false;
        }
    }
    return true;
}

bool ParseSettings(const Json& value, RuleSettings& settings,
                   const std::string& ruleId, std::string& error)
{
    if (!value.is_object())
    {
        error = "rule '" + ruleId + "' must be an object";
        return false;
    }
    for (auto item = value.begin(); item != value.end(); ++item)
    {
        const auto& key = item.key();
        if (key != "enabled" && key != "severity" && key != "options" && key != "paths")
        {
            error = "rule '" + ruleId + "' has unknown setting '" + key + "'";
            return false;
        }
    }
    if (value.contains("enabled"))
    {
        if (!value["enabled"].is_boolean())
        {
            error = "rule '" + ruleId + "' setting 'enabled' must be boolean";
            return false;
        }
        settings.enabled = value["enabled"].get<bool>();
    }
    if (value.contains("severity"))
    {
        if (!value["severity"].is_string() ||
            !ParseSeverity(value["severity"].get<std::string>(), settings.severity))
        {
            error = "rule '" + ruleId + "' has invalid severity";
            return false;
        }
    }
    if (value.contains("options") &&
        !ParseOptions(value["options"], settings.options, ruleId, error)) return false;
    return true;
}
}

RuleConfig RuleConfig::Load(const std::filesystem::path& path, std::string& error)
{
    RuleConfig config;
    if (path.empty())
    {
        return config;
    }

    std::ifstream input(path);
    if (!input)
    {
        error = "cannot open config file '" + path.string() + "'";
        return config;
    }

    try
    {
        const auto document = Json::parse(input);
        if (!document.is_object() || !document.contains("rules") ||
            !document["rules"].is_object())
        {
            error = "config must contain an object named 'rules'";
            return config;
        }

        for (const auto& [ruleId, value] : document["rules"].items())
        {
            RuleSettings settings;
            if (!ParseSettings(value, settings, ruleId, error)) return config;
            config.rules_[ruleId] = settings;

            if (value.contains("paths"))
            {
                if (!value["paths"].is_object())
                {
                    error = "rule '" + ruleId + "' setting 'paths' must be an object";
                    return config;
                }
                for (const auto& [pattern, pathValue] : value["paths"].items())
                {
                    if (pathValue.contains("paths"))
                    {
                        error = "rule '" + ruleId + "' path overrides cannot contain 'paths'";
                        return config;
                    }
                    RuleSettings pathSettings = settings;
                    if (!ParseSettings(pathValue, pathSettings, ruleId, error)) return config;
                    config.pathOverrides_[ruleId].push_back({pattern, pathSettings});
                }
            }
        }
    }
    catch (const Json::exception& exception)
    {
        error = "cannot parse config file '" + path.string() + "': " + exception.what();
    }
    return config;
}

RuleSettings RuleConfig::SettingsFor(std::string_view ruleId) const
{
    const auto match = rules_.find(std::string(ruleId));
    return match == rules_.end() ? RuleSettings{} : match->second;
}

RuleSettings RuleConfig::SettingsFor(std::string_view ruleId,
                                     const std::filesystem::path& sourcePath) const
{
    const auto settings = SettingsFor(ruleId);
    const auto overrides = pathOverrides_.find(std::string(ruleId));
    if (overrides == pathOverrides_.end()) return settings;

    const auto normalizedPath = sourcePath.generic_string();
    for (const auto& pathOverride : overrides->second)
    {
        if (MatchesPattern(normalizedPath, pathOverride.pattern)) return pathOverride.settings;
    }
    return settings;
}

std::vector<RuleSettings> RuleConfig::AllSettingsFor(std::string_view ruleId) const
{
    const auto id = std::string(ruleId);
    const auto base = rules_.find(id);
    std::vector<RuleSettings> result{
        base == rules_.end() ? RuleSettings{} : base->second};
    const auto overrides = pathOverrides_.find(id);
    if (overrides != pathOverrides_.end())
    {
        for (const auto& pathOverride : overrides->second)
        {
            result.push_back(pathOverride.settings);
        }
    }
    return result;
}

bool RuleConfig::HasRule(std::string_view ruleId) const
{
    return rules_.contains(std::string(ruleId));
}

std::vector<std::string> RuleConfig::RuleIds() const
{
    std::vector<std::string> ids;
    ids.reserve(rules_.size());
    for (const auto& [id, settings] : rules_) ids.push_back(id);
    return ids;
}
}
