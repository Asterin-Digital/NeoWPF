#pragma once

#include <NeoLint/Core/RuleSeverity.h>
#include <NeoLint/Core/RuleOptions.h>

#include <filesystem>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace neolint
{
struct RuleSettings
{
    bool enabled = true;
    RuleSeverity severity = RuleSeverity::Warning;
    RuleOptions options;
};

struct RulePathOverride
{
    std::string pattern;
    RuleSettings settings;
};

class RuleConfig
{
public:
    static RuleConfig Load(const std::filesystem::path& path, std::string& error);
    RuleSettings SettingsFor(std::string_view ruleId) const;
    RuleSettings SettingsFor(std::string_view ruleId,
                             const std::filesystem::path& sourcePath) const;
    std::vector<RuleSettings> AllSettingsFor(std::string_view ruleId) const;
    bool HasRule(std::string_view ruleId) const;
    std::vector<std::string> RuleIds() const;

private:
    std::unordered_map<std::string, RuleSettings> rules_;
    std::unordered_map<std::string, std::vector<RulePathOverride>> pathOverrides_;
};
}
