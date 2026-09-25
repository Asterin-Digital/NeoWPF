#pragma once

#include <NeoLint/Core/RuleSeverity.h>

#include <filesystem>
#include <string>
#include <unordered_map>

namespace neolint
{
struct RuleSettings
{
    bool enabled = true;
    RuleSeverity severity = RuleSeverity::Warning;
};

class RuleConfig
{
public:
    static RuleConfig Load(const std::filesystem::path& path, std::string& error);
    RuleSettings SettingsFor(std::string_view ruleId) const;

private:
    std::unordered_map<std::string, RuleSettings> rules_;
};
}
