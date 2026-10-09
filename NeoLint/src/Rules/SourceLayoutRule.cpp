#include <NeoLint/Rules/SourceLayoutRule.h>
#include <NeoLint/Core/RuleRegistry.h>

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <ranges>
#include <string>
#include <string_view>

namespace neolint
{
namespace
{
std::string Lowercase(std::string value)
{
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char character) {
        return static_cast<char>(std::tolower(character));
    });
    return value;
}

}

std::string_view SourceLayoutRule::Id() const noexcept
{
    return "neowpf-source-layout";
}

std::string_view SourceLayoutRule::Description() const noexcept
{
    return "Reject header files under src and source files under include.";
}

RuleSeverity SourceLayoutRule::DefaultSeverity() const noexcept
{
    return RuleSeverity::Error;
}

bool SourceLayoutRule::ValidateOptions(
    const RuleOptions& options, std::string& error) const
{
    for (const auto& key : options.Keys())
    {
        const bool valid =
            (key == "src_forbidden_extensions" ||
             key == "include_forbidden_extensions") &&
            options.TypeOf(key) == RuleOptions::Type::StringList;
        if (valid) continue;
        error = "rule '" + std::string(Id()) + "' has invalid option '" + key + "'";
        return false;
    }
    return true;
}

void SourceLayoutRule::Run(
    const std::vector<std::filesystem::path>& sourceRoots,
    const RuleConfig& config, DiagnosticsSink& diagnostics)
{
    for (const auto& root : sourceRoots)
    {
        const auto rootName = Lowercase(root.filename().string());
        const bool headersForbidden = rootName == "src";
        const bool sourcesForbidden = rootName == "include";
        if (!headersForbidden && !sourcesForbidden) continue;

        std::error_code error;
        std::filesystem::recursive_directory_iterator entry(root, error), end;
        while (!error && entry != end)
        {
            const auto filePath = entry->path();
            if (entry->is_regular_file(error))
            {
                auto settings = config.SettingsFor(Id(), filePath);
                if (!config.HasRule(Id())) settings.severity = DefaultSeverity();
                const auto extension = Lowercase(filePath.extension().string());
                const auto forbiddenExtensions = settings.options.GetStrings(
                    headersForbidden ? "src_forbidden_extensions" :
                        "include_forbidden_extensions",
                    headersForbidden ? RuleOptions::StringList{".h"} :
                        RuleOptions::StringList{".cpp"});
                const bool forbidden = std::ranges::any_of(forbiddenExtensions,
                    [&](const std::string& item) { return Lowercase(item) == extension; });
                if (settings.enabled && forbidden)
                {
                    diagnostics.Report({std::string(Id()), settings.severity,
                        {filePath.generic_string(), 1, 1},
                        std::string(headersForbidden ?
                            "Header file is not allowed under src/: " :
                            "Source file is not allowed under include/: ") + filePath.string()});
                }
            }
            entry.increment(error);
        }
    }
}

namespace
{
const TreeRuleRegistrar<SourceLayoutRule> registrar;
}
}