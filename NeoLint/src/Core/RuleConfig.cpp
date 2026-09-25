#include <NeoLint/Core/RuleConfig.h>

#include <nlohmann/json.hpp>

#include <fstream>
#include <string>

namespace neolint
{
namespace
{
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

bool ParseSettings(const nlohmann::json& value, RuleSettings& settings,
                   const std::string& ruleId, std::string& error)
{
    if (!value.is_object())
    {
        error = "rule '" + ruleId + "' must be an object";
        return false;
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
        const auto document = nlohmann::json::parse(input);
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
                    RuleSettings pathSettings = settings;
                    if (!ParseSettings(pathValue, pathSettings, ruleId, error)) return config;
                    config.pathOverrides_[ruleId].push_back({pattern, pathSettings});
                }
            }
        }
    }
    catch (const nlohmann::json::exception& exception)
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
