#include <NeoLint/Rules/PlatformUtilityDuplicateRule.h>
#include <NeoLint/Core/RuleContext.h>
#include <NeoLint/Core/RuleRegistry.h>

#include <clang/ASTMatchers/ASTMatchers.h>

#include <array>
#include <cctype>
#include <map>
#include <ranges>
#include <string>
#include <string_view>
#include <utility>

namespace neolint
{
using namespace clang::ast_matchers;

namespace
{
struct UtilityAlias
{
    std::string_view name;
    std::string_view canonical;
};

constexpr std::array kUtilityAliases{
    UtilityAlias{"widetoutf8", "NeoWPF::Platform::WideToUtf8"},
    UtilityAlias{"utf8towide", "NeoWPF::Platform::Utf8ToWide"},
    UtilityAlias{"toutf8", "NeoWPF::Platform::WideToUtf8"},
    UtilityAlias{"fromutf8", "NeoWPF::Platform::Utf8ToWide"},
    UtilityAlias{"towidestring", "NeoWPF::Platform::Utf8ToWide"},
    UtilityAlias{"widestringtostring", "NeoWPF::Platform::WideToUtf8"},
    UtilityAlias{"trim", "NeoWPF::Platform::Trim"},
    UtilityAlias{"tolower", "NeoWPF::Platform::ToLower"},
    UtilityAlias{"toupper", "NeoWPF::Platform::ToUpper"},
    UtilityAlias{"normalizefileextension", "NeoWPF::Platform::NormalizeFileExtension"},
    UtilityAlias{"isasciialpha", "NeoWPF::Platform::IsAsciiAlpha"},
    UtilityAlias{"isasciidigit", "NeoWPF::Platform::IsAsciiDigit"},
    UtilityAlias{"split", "NeoWPF::Platform::Split"},
    UtilityAlias{"tryparseint", "NeoWPF::Platform::TryParseInt"},
    UtilityAlias{"tryparseuint32hex", "NeoWPF::Platform::TryParseUInt32Hex"},
};

std::string NormalizeName(std::string value)
{
    std::string normalized;
    normalized.reserve(value.size());
    for (const char character : value)
    {
        if (character != '_' && character != '-')
        {
            normalized.push_back(static_cast<char>(std::tolower(
                static_cast<unsigned char>(character))));
        }
    }
    return normalized;
}

}

std::string_view PlatformUtilityDuplicateRule::Id() const noexcept
{
    return "neowpf-platform-utility-duplicate";
}

std::string_view PlatformUtilityDuplicateRule::Description() const noexcept
{
    return "Detect local helpers that duplicate NeoWPF::Platform utilities.";
}

RuleSeverity PlatformUtilityDuplicateRule::DefaultSeverity() const noexcept
{
    return RuleSeverity::Warning;
}

bool PlatformUtilityDuplicateRule::ValidateOptions(
    const RuleOptions& options, std::string& error) const
{
    for (const auto& key : options.Keys())
    {
        const bool valid = (key == "aliases" &&
                options.TypeOf(key) == RuleOptions::Type::StringMap) ||
            (key == "ignored_names" &&
                options.TypeOf(key) == RuleOptions::Type::StringList) ||
            (key == "canonical_namespace" &&
                options.TypeOf(key) == RuleOptions::Type::String);
        if (valid) continue;
        error = "rule '" + std::string(Id()) + "' has invalid option '" + key + "'";
        return false;
    }
    return true;
}

void PlatformUtilityDuplicateRule::Configure(const RuleOptions& options)
{
    aliases_.clear();
    for (const auto& alias : kUtilityAliases)
    {
        aliases_.emplace(alias.name, alias.canonical);
    }
    for (auto& [name, target] : options.GetStringMap("aliases"))
    {
        aliases_.insert_or_assign(NormalizeName(std::move(name)), std::move(target));
    }
    ignoredNames_ = options.GetStrings("ignored_names");
    canonicalNamespace_ = options.GetString("canonical_namespace", "NeoWPF::Platform");
}

void PlatformUtilityDuplicateRule::RegisterMatchers(
    MatchFinder& finder, MatchFinder::MatchCallback& callback)
{
    finder.addMatcher(
        functionDecl(isDefinition(), unless(cxxMethodDecl()))
            .bind("platform-utility-duplicate"),
        &callback);
}

void PlatformUtilityDuplicateRule::OnMatch(
    const MatchFinder::MatchResult& result, RuleContext& context)
{
    const auto* function = result.Nodes.getNodeAs<clang::FunctionDecl>("platform-utility-duplicate");
    if (!function) return;

    const auto qualifiedName = function->getQualifiedNameAsString();
    if (qualifiedName.starts_with(canonicalNamespace_ + "::")) return;

    const auto normalizedName = NormalizeName(function->getNameAsString());
    if (std::ranges::find(ignoredNames_, normalizedName) != ignoredNames_.end()) return;
    const auto alias = aliases_.find(normalizedName);
    if (alias == aliases_.end()) return;

    context.Report(Id(), function->getLocation(), context.SettingsFor(Id()).severity,
                   "Local function '" + function->getNameAsString() +
                       "' duplicates " + alias->second);
}

namespace
{
const RuleRegistrar<PlatformUtilityDuplicateRule> registrar;
}
}
