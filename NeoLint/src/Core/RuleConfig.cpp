#include <NeoLint/Core/RuleConfig.h>

#include <fstream>
#include <string_view>

namespace neolint
{
namespace
{
RuleSeverity ParseSeverity(std::string_view value)
{
    if (value == "error") return RuleSeverity::Error;
    if (value == "info") return RuleSeverity::Info;
    return RuleSeverity::Warning;
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

    std::string line;
    std::string currentRule;
    while (std::getline(input, line))
    {
        const auto comment = line.find('#');
        if (comment != std::string::npos) line.erase(comment);
        const auto first = line.find_first_not_of(" \t");
        if (first == std::string::npos) continue;
        const auto colon = line.find(':', first);
        if (colon == std::string::npos) continue;

        const auto key = line.substr(first, colon - first);
        auto value = line.substr(colon + 1);
        const auto valueFirst = value.find_first_not_of(" \t");
        if (valueFirst == std::string::npos) continue;
        value.erase(0, valueFirst);
        if (value == "") continue;

        if (first == 0 && key == "rules") continue;
        if (first <= 2)
        {
            currentRule = key;
            config.rules_[currentRule] = {};
            continue;
        }
        if (currentRule.empty()) continue;
        if (key == "severity") config.rules_[currentRule].severity = ParseSeverity(value);
        if (key == "enabled") config.rules_[currentRule].enabled = value != "false";
    }
    return config;
}

RuleSettings RuleConfig::SettingsFor(std::string_view ruleId) const
{
    const auto match = rules_.find(std::string(ruleId));
    return match == rules_.end() ? RuleSettings{} : match->second;
}
}
