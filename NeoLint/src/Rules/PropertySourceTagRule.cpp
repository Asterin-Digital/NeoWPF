#include <NeoLint/Rules/PropertySourceTagRule.h>
#include <NeoLint/Core/RuleContext.h>
#include <NeoLint/Core/RuleRegistry.h>

#include <clang/ASTMatchers/ASTMatchers.h>

namespace neolint
{
using namespace clang::ast_matchers;

std::string_view PropertySourceTagRule::Id() const noexcept
{
    return "neowpf-property-source-tag";
}

std::string_view PropertySourceTagRule::Description() const noexcept
{
    return "Require trigger and style property wrappers to preserve their value source.";
}

RuleSeverity PropertySourceTagRule::DefaultSeverity() const noexcept
{
    return RuleSeverity::Warning;
}

void PropertySourceTagRule::RegisterMatchers(
    MatchFinder& finder, MatchFinder::MatchCallback& callback)
{
    finder.addMatcher(
        callExpr(
            callee(functionDecl(hasName("SetNamedPropertyValue"))),
            hasArgument(2, declRefExpr(to(enumConstantDecl(hasName("Local"))))),
            hasAncestor(cxxMethodDecl(anyOf(
                hasName("SetTriggerPropertyValue"),
                hasName("SetStylePropertyValue"))))
        ).bind("invalid-wrapper-source"), &callback);
}

void PropertySourceTagRule::OnMatch(const MatchFinder::MatchResult& result,
                                    RuleContext& context)
{
    const auto* call = result.Nodes.getNodeAs<clang::CallExpr>("invalid-wrapper-source");
    if (!call) return;
    context.Report(Id(), call->getExprLoc(), context.SettingsFor(Id()).severity,
                   "SetTriggerPropertyValue and SetStylePropertyValue must not pass "
                   "PropertyValueSource::Local");
}

namespace
{
const RuleRegistrar<PropertySourceTagRule> registrar;
}
}
