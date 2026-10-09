#include <NeoLint/Rules/ForbiddenNamespaceRule.h>
#include <NeoLint/Core/RuleContext.h>
#include <NeoLint/Core/RuleRegistry.h>

#include <clang/AST/Decl.h>
#include <clang/ASTMatchers/ASTMatchers.h>

#include <algorithm>
#include <array>
#include <cctype>
#include <string>
#include <string_view>

namespace neolint
{
using namespace clang::ast_matchers;

namespace
{
constexpr std::array<std::string_view, 4> kForbiddenComponents{
    "detail", "implementation", "private", "internal"};

std::string Lowercase(std::string value)
{
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char character) {
        return static_cast<char>(std::tolower(character));
    });
    return value;
}
}

std::string_view ForbiddenNamespaceRule::Id() const noexcept
{
    return "neowpf-forbidden-namespace";
}

std::string_view ForbiddenNamespaceRule::Description() const noexcept
{
    return "Forbid detail, implementation, private, and internal namespaces under NeoWPF.";
}

RuleSeverity ForbiddenNamespaceRule::DefaultSeverity() const noexcept
{
    return RuleSeverity::Error;
}

void ForbiddenNamespaceRule::RegisterMatchers(
    MatchFinder& finder, MatchFinder::MatchCallback& callback)
{
    finder.addMatcher(namespaceDecl().bind("forbidden-namespace"), &callback);
}

void ForbiddenNamespaceRule::OnMatch(
    const MatchFinder::MatchResult& result, RuleContext& context)
{
    const auto* declaration = result.Nodes.getNodeAs<clang::NamespaceDecl>(
        "forbidden-namespace");
    if (!declaration) return;

    const auto qualifiedName = declaration->getQualifiedNameAsString();
    if (!qualifiedName.starts_with("NeoWPF::")) return;

    const auto component = Lowercase(declaration->getNameAsString());
    const auto forbidden = std::find_if(kForbiddenComponents.begin(),
        kForbiddenComponents.end(), [&](std::string_view word) {
            return component.find(word) != std::string::npos;
        });
    if (forbidden == kForbiddenComponents.end()) return;

    context.Report(Id(), declaration->getLocation(), context.SettingsFor(Id()).severity,
        "Namespace '" + qualifiedName + "' contains forbidden term '" +
        std::string(*forbidden) + "'");
}

namespace
{
const RuleRegistrar<ForbiddenNamespaceRule> registrar;
}
}