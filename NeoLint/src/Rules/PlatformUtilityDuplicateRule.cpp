#include <NeoLint/Rules/PlatformUtilityDuplicateRule.h>
#include <NeoLint/Core/RuleContext.h>
#include <NeoLint/Core/RuleRegistry.h>

#include <clang/ASTMatchers/ASTMatchers.h>

#include <array>
#include <cctype>
#include <string>
#include <string_view>

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

const UtilityAlias* FindAlias(std::string_view name)
{
    for (const auto& alias : kUtilityAliases)
    {
        if (alias.name == name) return &alias;
    }
    return nullptr;
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

void PlatformUtilityDuplicateRule::RegisterMatchers(
    MatchFinder& finder, MatchFinder::MatchCallback& callback)
{
    finder.addMatcher(
        functionDecl(isDefinition(), unless(cxxMethodDecl())).bind("platform-utility-duplicate"),
        &callback);
}

void PlatformUtilityDuplicateRule::OnMatch(
    const MatchFinder::MatchResult& result, RuleContext& context)
{
    const auto* function = result.Nodes.getNodeAs<clang::FunctionDecl>("platform-utility-duplicate");
    if (!function || !context.SourceMgr().isInMainFile(function->getLocation())) return;

    const auto qualifiedName = function->getQualifiedNameAsString();
    if (qualifiedName.starts_with("NeoWPF::Platform::")) return;

    const auto normalizedName = NormalizeName(function->getNameAsString());
    const auto* alias = FindAlias(normalizedName);
    if (!alias) return;

    context.Report(Id(), function->getLocation(), context.SettingsFor(Id()).severity,
                   "Local function '" + function->getNameAsString() +
                       "' duplicates " + std::string(alias->canonical));
}

namespace
{
const RuleRegistrar<PlatformUtilityDuplicateRule> registrar;
}
}
