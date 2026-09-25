#include <NeoLint/Rules/DPRegistrationLocationRule.h>
#include <NeoLint/Core/RuleContext.h>
#include <NeoLint/Core/RuleRegistry.h>

#include <clang/ASTMatchers/ASTMatchers.h>

#include <string>

namespace neolint
{
using namespace clang::ast_matchers;

std::string_view DPRegistrationLocationRule::Id() const noexcept
{
    return "neowpf-dp-registration-location";
}

std::string_view DPRegistrationLocationRule::Description() const noexcept
{
    return "Keep dependency-property registration in dedicated *Properties.cpp files.";
}

RuleSeverity DPRegistrationLocationRule::DefaultSeverity() const noexcept
{
    return RuleSeverity::Warning;
}

void DPRegistrationLocationRule::RegisterMatchers(
    MatchFinder& finder, MatchFinder::MatchCallback& callback)
{
    finder.addMatcher(
        callExpr(
            callee(functionDecl(anyOf(
                hasName("RegisterProperty"),
                hasName("RegisterAttachedProperty"),
                hasName("RegisterStaticPropertyHandle"),
                    allOf(hasAnyName("Register", "RegisterAttached"),
                        cxxMethodDecl(ofClass(cxxRecordDecl(hasName("DependencyProperty"))))))))
        ).bind("dependency-property-registration"), &callback);
}

void DPRegistrationLocationRule::OnMatch(
    const MatchFinder::MatchResult& result, RuleContext& context)
{
    const auto* call = result.Nodes.getNodeAs<clang::CallExpr>(
        "dependency-property-registration");
    if (!call) return;

    if (!context.SourceMgr().isInMainFile(call->getExprLoc())) return;

    const auto presumed = context.SourceMgr().getPresumedLoc(call->getExprLoc());
    if (presumed.isInvalid()) return;

    const std::string filename = presumed.getFilename();
    constexpr std::string_view propertiesSuffix = "Properties.cpp";
    if (filename.size() >= propertiesSuffix.size() &&
        filename.ends_with(propertiesSuffix))
    {
        return;
    }

    context.Report(Id(), call->getExprLoc(), context.SettingsFor(Id()).severity,
                   "Dependency-property registration must be in a *Properties.cpp file");
}

namespace
{
const RuleRegistrar<DPRegistrationLocationRule> registrar;
}
}
